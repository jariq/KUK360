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
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using KUK360.Codes;

namespace KUK360.Viewers
{
    /// <summary>
    /// Flat projection image viewer
    /// </summary>
    public partial class FlatViewer : UserControl
    {
        #region Private fields

        /// <summary>
        /// Full path to viewed image
        /// </summary>
        private string _imagePath = null;

        /// <summary>
        /// WPF image control
        /// </summary>
        private Image _imageControl = null;

        /// <summary>
        /// Bitmap image to be viewed
        /// </summary>
        private BitmapImage _bitmapImage = null;

        /// <summary>
        /// Image rotation
        /// </summary>
        private Rotation _imageRotation = Rotation.Rotate0;

        /// <summary>
        /// Flag indicating whether image should fit canvas when window is resized
        /// </summary>
        private bool _shouldFitCanvas = true;

        /// <summary>
        /// Flag indicating whether mouse button is pressed down
        /// </summary>
        private bool _leftMouseDown = false;

        /// <summary>
        /// Point where the mouse was captured the last time
        /// </summary>
        private Point _lastMousePosition = new Point(0, 0);

        #endregion

        #region Constructor

        /// <summary>
        /// Class constructor
        /// </summary>
        public FlatViewer()
        {
            InitializeComponent();

            Reset(true);
        }

        #endregion

        #region Image loading

        /// <summary>
        /// Resets control
        /// </summary>
        /// <param name="resetRotation">Flag indicating whether image rotation should be reset too</param>
        private void Reset(bool resetRotation)
        {
            // Reset private fields
            _imagePath = null;
            _bitmapImage = null;
            if (resetRotation)
                _imageRotation = Rotation.Rotate0;
            _shouldFitCanvas = true;
            _leftMouseDown = false;
            _lastMousePosition = new Point(0, 0);

            // Reset cursor
            this.Cursor = Cursors.Arrow;

            // Setup new image control
            _imageControl = new Image();
            RenderOptions.SetBitmapScalingMode(_imageControl, BitmapScalingMode.HighQuality);
            _imageControl.SizeChanged += ImageDisplayedFirstTime;
            flatCanvas.Children.Clear();
            flatCanvas.Children.Add(_imageControl);
            Canvas.SetLeft(_imageControl, 0);
            Canvas.SetTop(_imageControl, 0);
            _imageControl.RenderTransform = Transform.Identity;
        }

        /// <summary>
        /// Fired when image is displayed for the first time after it is loaded
        /// </summary>
        /// <param name="sender"></param>
        /// <param name="e"></param>
        private void ImageDisplayedFirstTime(object sender, SizeChangedEventArgs e)
        {
            _imageControl.SizeChanged -= ImageDisplayedFirstTime;
            FitCanvas();
        }

        /// <summary>
        /// Loads and views equirectangular image in the sphere
        /// </summary>
        /// <param name="path">Path to the image file</param>
        public void LoadImage(string path, bool resetRotation)
        {
            Reset(resetRotation);

            if (string.IsNullOrEmpty(path))
                return;

            _imagePath = path;

            try
            {
                // var stopWatch = System.Diagnostics.Stopwatch.StartNew();

                // Load image as bitmap
                // Note: The same image is loaded in SphereViewer first (takes > 100 ms) and then in FlatViewer (takes 0 ms)
                _bitmapImage = new BitmapImage();
                _bitmapImage.BeginInit();
                _bitmapImage.UriSource = new Uri(_imagePath);
                _bitmapImage.CacheOption = BitmapCacheOption.OnLoad;
                _bitmapImage.Rotation = _imageRotation;
                _bitmapImage.EndInit();

                // stopWatch.Stop();
                // System.Diagnostics.Trace.WriteLine("FlatViewer loading took " + stopWatch.ElapsedMilliseconds + "ms");
            }
            catch (Exception ex)
            {
                throw new ImageLoadingException(path, ex);
            }

            // Pass loaded bitmap to image control
            _imageControl.Source = _bitmapImage;
        }

        #endregion

        #region Rotation

        /// <summary>
        /// Moves rotation to left and reloads image
        /// </summary>
        public void RotateLeft()
        {
            switch (_imageRotation)
            {
                case Rotation.Rotate0:
                    _imageRotation = Rotation.Rotate270;
                    break;
                case Rotation.Rotate90:
                    _imageRotation = Rotation.Rotate0;
                    break;
                case Rotation.Rotate180:
                    _imageRotation = Rotation.Rotate90;
                    break;
                case Rotation.Rotate270:
                    _imageRotation = Rotation.Rotate180;
                    break;
            }

            LoadImage(_imagePath, false);
        }

        /// <summary>
        /// Moves rotation to right and reloads image
        /// </summary>
        public void RotateRight()
        {
            switch (_imageRotation)
            {
                case Rotation.Rotate0:
                    _imageRotation = Rotation.Rotate90;
                    break;
                case Rotation.Rotate90:
                    _imageRotation = Rotation.Rotate180;
                    break;
                case Rotation.Rotate180:
                    _imageRotation = Rotation.Rotate270;
                    break;
                case Rotation.Rotate270:
                    _imageRotation = Rotation.Rotate0;
                    break;
            }

            LoadImage(_imagePath, false);
        }

        #endregion

        #region Canvas fitting

        private void FlatCanvas_SizeChanged(object sender, SizeChangedEventArgs e)
        {
            FitCanvas();
        }

        private void FitCanvas()
        {
            if (!_shouldFitCanvas)
                return;

            Size imageSize = _imageControl.RenderSize;
            Size canvasSize = flatCanvas.RenderSize;

            // Image needs to be stretched
            if ((imageSize.Width > canvasSize.Width) || (imageSize.Height > canvasSize.Height))
            {
                // Determine stretch ratio
                double widthRatio = canvasSize.Width / imageSize.Width;
                double heightRatio = canvasSize.Height / imageSize.Height;
                double applyRatio = (widthRatio < heightRatio) ? widthRatio : heightRatio;

                // Apply stretch ratio as zoom value
                ApplyZoom(applyRatio * 100);

                // Determine image size after transformation
                Size transformedImageSize = new Size(imageSize.Width * applyRatio, imageSize.Height * applyRatio);

                // Move image to center
                Canvas.SetLeft(_imageControl, (canvasSize.Width - transformedImageSize.Width) / 2);
                Canvas.SetTop(_imageControl, (canvasSize.Height - transformedImageSize.Height) / 2);
            }
            // Image does not need to be stretched
            else
            {
                ApplyZoom(100);

                // Move image to center
                Canvas.SetLeft(_imageControl, (canvasSize.Width - imageSize.Width) / 2);
                Canvas.SetTop(_imageControl, (canvasSize.Height - imageSize.Height) / 2);
            }
        }

        #endregion

        #region ZoomControl

        /// <summary>
        /// Toogle zoom control display
        /// </summary>
        public void ToggleZoomControl()
        {
            if (this.zoomControl.Visibility == Visibility.Visible)
            {
                HideZoomControl();
            }
            else
            {
                ShowZoomControl();
            }
        }

        /// <summary>
        /// Display zoom control
        /// </summary>
        public void ShowZoomControl()
        {
            this.zoomControl.Visibility = Visibility.Visible;
        }

        /// <summary>
        /// Hide zoom control
        /// </summary>
        public void HideZoomControl()
        {
            this.zoomControl.Visibility = Visibility.Hidden;
        }

        /// <summary>
        /// Zoom value changed via zoom control
        /// </summary>
        /// <param name="zoomValue"></param>
        private void zoomControl_ZoomValueChanged(double zoomValue)
        {
            ApplyZoom(zoomValue);
        }

        #endregion

        #region Image moving

        /// <summary>
        /// Left mouse button is pressed
        /// </summary>
        /// <param name="sender">Who knows..</param>
        /// <param name="e">Who knows..</param>
        private void FlatCanvas_MouseLeftButtonDown(object sender, MouseButtonEventArgs e)
        {
            _leftMouseDown = true;

            this.Cursor = Cursors.Hand;
            _lastMousePosition = e.GetPosition(this);
            _imageControl.CaptureMouse();
        }

        /// <summary>
        /// Mouse is moved
        /// </summary>
        /// <param name="sender">Who knows..</param>
        /// <param name="e">Who knows..</param>
        private void FlatCanvas_MouseMove(object sender, MouseEventArgs e)
        {
            // Do nothing if left button is not pressed
            if (!_leftMouseDown)
                return;

            _shouldFitCanvas = false;

            // Determine mouse distance
            Point currentMousePosition = e.GetPosition(this);
            Point mouseDistance = new Point(_lastMousePosition.X - currentMousePosition.X, _lastMousePosition.Y - currentMousePosition.Y);

            // Change image control position in canvas
            Canvas.SetLeft(_imageControl, Canvas.GetLeft(_imageControl) - mouseDistance.X);
            Canvas.SetTop(_imageControl, Canvas.GetTop(_imageControl) - mouseDistance.Y);

            _lastMousePosition = currentMousePosition;
        }

        /// <summary>
        /// Left mouse button is released
        /// </summary>
        /// <param name="sender">Who knows..</param>
        /// <param name="e">Who knows..</param>
        private void FlatCanvas_MouseUp(object sender, MouseButtonEventArgs e)
        {
            _leftMouseDown = false;

            this.Cursor = Cursors.Arrow;
            _imageControl.ReleaseMouseCapture();
        }

        #endregion

        #region Zooming

        /// <summary>
        /// Mouse wheel is turned
        /// </summary>
        /// <param name="sender">Who knows..</param>
        /// <param name="e">Who knows..</param>
        private void FlatCanvas_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            _shouldFitCanvas = false;

            if (e.Delta >= 0)
            {
                ApplyZoomAtPoint(zoomControl.ZoomValue + 5, e.GetPosition(_imageControl));
            }
            else
            {
                ApplyZoomAtPoint(zoomControl.ZoomValue - 5, e.GetPosition(_imageControl));
            }
        }

        /// <summary>
        /// Zoom with mouse position
        /// </summary>
        /// <param name="zoom">Zoom value</param>
        /// <param name="point">Mouse position</param>
        private void ApplyZoomAtPoint(double zoom, Point point)
        {
            zoomControl.ZoomValue = zoom; // Note: This also normalizes value

            Matrix matrix = _imageControl.RenderTransform.Value;

            // Formula: expectedValue = currentValue * scale;
            double currentValue = matrix.M11;
            double expectedValue = zoomControl.ZoomValue / 100;
            double scale = expectedValue / currentValue;

            matrix.ScaleAtPrepend(scale, scale, point.X, point.Y);
            _imageControl.RenderTransform = new MatrixTransform(matrix);
        }

        /// <summary>
        /// Zoom without mouse position
        /// </summary>
        /// <param name="zoom">Zoom value</param>
        private void ApplyZoom(double zoom)
        {
            zoomControl.ZoomValue = zoom; // Note: This also normalizes value

            Matrix matrix = _imageControl.RenderTransform.Value;

            matrix.M11 = zoomControl.ZoomValue / 100;
            matrix.M22 = zoomControl.ZoomValue / 100;

            _imageControl.RenderTransform = new MatrixTransform(matrix);

            // TODO - Center or something ???
        }

        /// <summary>
        /// Zoom in from the outside
        /// </summary>
        public void ZoomIn()
        {
            _shouldFitCanvas = false;

            ApplyZoom(zoomControl.ZoomValue + 5);
        }

        /// <summary>
        /// Zoom out from the outside
        /// </summary>
        public void ZoomOut()
        {
            _shouldFitCanvas = false;

            ApplyZoom(zoomControl.ZoomValue - 5);
        }

        /// <summary>
        /// Reset zoom from the outside
        /// </summary>
        public void ZoomReset()
        {
            _shouldFitCanvas = false;

            ApplyZoom(100);
        }

        #endregion
    }
}
