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

using System.Windows;
using System.Windows.Controls;

namespace KUK360.Controls
{
    /// <summary>
    /// Visual zoom control with slider
    /// </summary>
    public partial class ZoomControl : UserControl
    {
        /// <summary>
        /// Minimum zoom value
        /// </summary>
        public double Minimum
        {
            get
            {
                return zoomSlider.Minimum;
            }
            set
            {
                zoomSlider.Minimum = value;
            }
        }

        /// <summary>
        /// Maximum zoom value
        /// </summary>
        public double Maximum
        {
            get
            {
                return zoomSlider.Maximum;
            }
            set
            {
                zoomSlider.Maximum = value;
            }
        }

        /// <summary>
        /// Current zoom value
        /// </summary>
        public double ZoomValue
        {
            get
            {
                return zoomSlider.Value;
            }
            set
            {
                double normalizedValue;

                if (value > zoomSlider.Maximum)
                {
                    normalizedValue = zoomSlider.Maximum;
                }
                else if (value < zoomSlider.Minimum)
                {
                    normalizedValue = zoomSlider.Minimum;
                }
                else
                {
                    normalizedValue = value;
                }

                zoomSlider.ValueChanged -= ZoomSlider_ValueChanged;
                zoomSlider.Value = normalizedValue;
                zoomSlider.ValueChanged += ZoomSlider_ValueChanged;
            }
        }

        /// <summary>
        /// Class constructor
        /// </summary>
        public ZoomControl()
        {
            InitializeComponent();

            zoomSlider.ValueChanged += ZoomSlider_ValueChanged;
        }

        /// <summary>
        /// Delegate for event that occurs when zoom value gets changed via slider
        /// </summary>
        /// <param name="zoomValue">Current zoom value</param>
        public delegate void ZoomValueChangedDelegate(double zoomValue);

        /// <summary>
        /// Public event that occurs when zoom value gets changed via slider
        /// </summary>
        public event ZoomValueChangedDelegate ZoomValueChanged;

        /// <summary>
        /// Private handler called when zoom value gets changed via slider
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ZoomSlider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            ZoomValueChanged?.Invoke(e.NewValue);
        }
    }
}
