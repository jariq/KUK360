/* MIT License
 * 
 * KUK360 - Simple 360 photo viewer for Windows
 * Copyright (c) 2019-2024 Jaroslav Imrich <jimrich@jimrich.sk>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

using System;
using System.ComponentModel;
using System.Windows;
using System.Windows.Threading;
using KUK360.Codes;

namespace KUK360.Windows
{
    /// <summary>
    /// Simple dialog window that executes user defined Action in a BackgroundWorker and displays progress bar
    /// </summary>
    public partial class WaitWindow : Window
    {
        private static WaitWindow _instance = null;
        private static object _instanceLock = new object();

        private BackgroundWorker _backgroundWorker = null;
        private Action _backgroundWorkerAction = null;
        private Exception _backgroundWorkerException = null;
        private bool _backgroundWorkerFinished = false;

        private bool _canClose = false;

        private DispatcherTimer _timer = null;
        private int _timerSeconds = 1;
        private uint _minWindowDisplayTime = 0;

        // Private constructor prevents incorrect usage.
        // WaitWindow.Execute method has to be used to display this window.
        private WaitWindow()
        {
            InitializeComponent();

            // Only a single instance of WaitWindow may exist in a whole application
            if (_instance != null)
                throw new Exception("WaitWindow instance already exists");

            // Setup BackgroundWorker that executes user defined Action
            _backgroundWorker = new BackgroundWorker();
            _backgroundWorker.WorkerReportsProgress = false;
            _backgroundWorker.WorkerSupportsCancellation = false;
            _backgroundWorker.DoWork += BackgroundWorkerDoWork;
            _backgroundWorker.RunWorkerCompleted += BackgroundWorkerCompleted;

            // Setup event handler that will execute BackgroundWorker once the window is displayed
            this.Loaded += WaitWindowLoaded;

            // Setup timer that keeps the track of window display time and closes the window
            _timer = new DispatcherTimer();
            _timer.Interval = TimeSpan.FromMilliseconds(1000);
            _timer.Tick += TimerTick;
            _timer.Start();
        }

        private void TimerTick(object sender, EventArgs e)
        {
            // Keep the track of window display time
            _timerSeconds++;

            // Check once in a second whether the BackgroundWorker finished
            // and wait for defined number of seconds in case BackgroundWorker finished too soon
            if (_backgroundWorkerFinished && _timerSeconds > _minWindowDisplayTime)
            {
                // Stop the timer
                _timer.Stop();

                // Let other methods know that this window might be closed now
                _canClose = true;

                // Close the window
                _instance.Close();
            }
        }

        protected override void OnClosing(CancelEventArgs e)
        {
            // Prevent user from manually closing the window
            e.Cancel = !_canClose;
        }

        private void WaitWindowLoaded(object sender, RoutedEventArgs e)
        {
            // Run BackgroundWorker once the window is displayed
            _instance._backgroundWorker.RunWorkerAsync();
        }

        private void BackgroundWorkerDoWork(object sender, DoWorkEventArgs e)
        {
            // User defined Action is executed in BackgroundWorker
            _backgroundWorkerAction();
        }

        private void BackgroundWorkerCompleted(object sender, RunWorkerCompletedEventArgs e)
        {
            // Keep any exception that occurred in user defined Action
            _backgroundWorkerException = e.Error;
            // Let other methods know that BackgroundWorker is finished
            _backgroundWorkerFinished = true;
        }

        /// <summary>
        /// Displays WaitWindow as a dialog window and executes user defined Action in a BackgroundWorker
        /// </summary>
        /// <param name="owner">Window that owns WaitWindow</param>
        /// <param name="action">User defined Action that will be executed in a BackgroundWorker</param>
        /// <param name="minWindowDisplayTime">Minimum duration, in seconds, for which the WaitWindow will be displayed</param>
        /// <exception cref="ArgumentNullException">Thrown when required parameter is null</exception>
        /// <exception cref="BackgroundWorkerException">Thrown when exception occurs in user defined Action</exception>
        public static void Execute(Window owner, Action action, uint minWindowDisplayTime)
        {
            if (owner == null)
                throw new ArgumentNullException("owner");

            if (action == null)
                throw new ArgumentNullException("action");

            // Only a single instance of WaitWindow may exist in a whole application
            lock (_instanceLock)
            {
                // Create new WaitWindow instance
                _instance = new WaitWindow();

                // Setup WaitWindow
                _instance.Owner = owner;
                _instance._backgroundWorkerAction = action;
                _instance._minWindowDisplayTime = minWindowDisplayTime;

                // Show WaitWindow as a dialog window
                // Note: This is a blocking method.
                // Note: BackgroundWorker with user defined Action is executed by an event handler once the window is displayed.
                // Note: Window is closed by a timer that keeps the track of window display time and BackgroundWorker activity.
                _instance.ShowDialog();

                // Keep exception that might have occurred in user defined Action
                Exception backgroundWorkerException = _instance._backgroundWorkerException;

                // Dispose BackgroundWorker
                _instance._backgroundWorker.Dispose();
                _instance._backgroundWorker = null;

                // Forget this already closed WaitWindow instance
                _instance = null;

                // If needed throw BackgroundWorkerException with an exception that occurred in user defined Action
                if (backgroundWorkerException != null)
                    throw new BackgroundWorkerException(backgroundWorkerException);
            }
        }
    }
}
