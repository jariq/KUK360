﻿/* MIT License
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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Threading;

namespace KUK360.Viewers
{
    public partial class BubbleViewer : UserControl
    {
        private DispatcherTimer _timer = null;

        public BubbleViewer()
        {
            InitializeComponent();

            _timer = new DispatcherTimer();
            _timer.Interval = TimeSpan.FromMilliseconds(1000);
            _timer.Tick += Timer_Tick;

            HideBubble();
        }

        private void Timer_Tick(object sender, EventArgs e)
        {
            _timer.Stop();

            borderBubbleText.Visibility = Visibility.Collapsed;
        }

        public void HideBubble()
        {
            borderBubbleText.Visibility = Visibility.Collapsed;
        }

        public void ShowTemporaryBubble(string text)
        {
            _timer.Stop();

            labelBubbleText.Content = text;
            
            borderBubbleText.Visibility = Visibility.Visible;
            
            _timer.Start();
        }

        public void ShowPermanentBubble(string text)
        {
            _timer.Stop();

            labelBubbleText.Content = text;

            borderBubbleText.Visibility = Visibility.Visible;
        }
    }
}
