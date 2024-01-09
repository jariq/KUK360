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

using KUK360.Windows;
using System.Windows;

namespace KUK360.Codes
{
    internal class FullScreenManager
    {
        private MainWindow _mainWindow = null;

        private WindowState _lastWindowState;
        private GridLength _gridLengthRow0;
        private GridLength _gridLengthRow2;

        public bool IsInFullScreenMode
        {
            get;
            private set;
        }

        public FullScreenManager(MainWindow mainWindow)
        {
            _mainWindow = mainWindow;
            IsInFullScreenMode = false;
        }

        public void ToggleFullScreenMode()
        {
            if (!IsInFullScreenMode)
                EnterFullScreenMode();
            else
                ExitFullScreenMode();
        }

        public void EnterFullScreenMode()
        {
            if (!IsInFullScreenMode)
            {
                _lastWindowState = _mainWindow.WindowState;

                _mainWindow.Visibility = Visibility.Collapsed;
                _mainWindow.WindowStyle = WindowStyle.None;
                _mainWindow.ResizeMode = ResizeMode.NoResize;
                _mainWindow.WindowState = WindowState.Maximized;

                _gridLengthRow0 = _mainWindow.gridMain.RowDefinitions[0].Height;
                _mainWindow.gridMain.RowDefinitions[0].Height = new GridLength(0);

                _gridLengthRow2 = _mainWindow.gridMain.RowDefinitions[2].Height;
                _mainWindow.gridMain.RowDefinitions[2].Height = new GridLength(0);

                _mainWindow.Topmost = true;
                _mainWindow.Visibility = Visibility.Visible;

                IsInFullScreenMode = true;
            }
        }

        public void ExitFullScreenMode()
        {
            if (IsInFullScreenMode)
            {
                _mainWindow.Visibility = Visibility.Collapsed;
                _mainWindow.WindowStyle = WindowStyle.SingleBorderWindow;
                _mainWindow.ResizeMode = ResizeMode.CanResize;
                _mainWindow.WindowState = _lastWindowState;

                _mainWindow.gridMain.RowDefinitions[0].Height = _gridLengthRow0;
                _mainWindow.gridMain.RowDefinitions[2].Height = _gridLengthRow2;

                _mainWindow.Topmost = false;
                _mainWindow.Visibility = Visibility.Visible;

                IsInFullScreenMode = false;
            }
        }
    }
}
