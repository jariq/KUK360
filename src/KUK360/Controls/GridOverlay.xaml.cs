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
using System.Windows.Media;
using System.Windows.Shapes;

namespace KUK360.Controls
{
    /// <summary>
    /// Transparent green grid to be used as an image overlay
    /// </summary>
    public partial class GridOverlay : UserControl
    {
        /// <summary>
        /// Spacing between the lines in pixels
        /// </summary>
        private int _lineSpacing = 40;

        /// <summary>
        /// Line color no.1
        /// </summary>
        private SolidColorBrush _lineColor1 = new SolidColorBrush(Color.FromArgb(0xFF, 0x80, 0xFF, 0x00));

        /// <summary>
        /// Line color no.2
        /// </summary>
        private SolidColorBrush _lineColor2 = new SolidColorBrush(Color.FromArgb(0xFF, 0x53, 0xED, 0xF3));

        /// <summary>
        /// Class constructor
        /// </summary>
        public GridOverlay()
        {
            InitializeComponent();
            GenerateGrid();
        }

        /// <summary>
        /// Generates grid lines
        /// </summary>
        private void GenerateGrid()
        {
            // Grid size is the same as the size of all screens combined
            gridCanvas.Width = SystemParameters.VirtualScreenWidth;
            gridCanvas.Height = SystemParameters.VirtualScreenHeight;

            bool useColor2 = false;

            // Generate vertical lines
            int x = _lineSpacing;
            while (x < gridCanvas.Width)
            {
                Line line = new Line()
                {
                    X1 = x,
                    Y1 = 0,
                    X2 = x,
                    Y2 = gridCanvas.Height,
                    SnapsToDevicePixels = true,
                    StrokeThickness = 1,
                    Stroke = useColor2 ? _lineColor2 : _lineColor1
                };

                gridCanvas.Children.Add(line);

                x += _lineSpacing;
                useColor2 = !useColor2;
            };

            // Generate horizontal lines
            int y = _lineSpacing;
            while (y < gridCanvas.Height)
            {
                Line line = new Line()
                {
                    X1 = 0,
                    Y1 = y,
                    X2 = gridCanvas.Width,
                    Y2 = y,
                    SnapsToDevicePixels = true,
                    StrokeThickness = 1,
                    Stroke = useColor2 ? _lineColor2 : _lineColor1
                };

                gridCanvas.Children.Add(line);

                y += _lineSpacing;
                useColor2 = !useColor2;
            };
        }
    }
}
