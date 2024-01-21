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

/*
 * The code in this file was inspired by an external project.
 * 
 * Original project: PanoDotNet
 * Original repository: https://github.com/hajduakos/PanoDotNet/tree/69242a7fd29d1733f8a764aa67edc0a9ac0233a6
 * Original license: MIT
 * 
 * MIT License
 * 
 * Copyright (c) 2017 Akos Hajdu
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
using System.Windows.Media.Media3D;
using System.Windows.Threading;

namespace KUK360.Viewers
{
    /// <summary>
    /// 360 projection image viewer
    /// </summary>
    public partial class SphereViewer : UserControl
    {
        #region Private fields

        /// <summary>
        /// Image to be viewed
        /// </summary>
        private BitmapImage _image = null;

        /// <summary>
        /// Tessellated sphere mesh
        /// </summary>
        private MeshGeometry3D _sphereMesh = null;

        /// <summary>
        /// Brush containing the panorama
        /// </summary>
        private ImageBrush _brush = null;

        /// <summary>
        /// Camera horizontal orientation
        /// </summary>
        private double _camTheta = 180;

        /// <summary>
        /// Camera vertical orientation
        /// </summary>
        private double _camPhi = 90;

        /// <summary>
        /// Camera horizontal movement speed
        /// </summary>
        private double _camThetaSpeed = 0;

        /// <summary>
        /// Camera vertical movement speed
        /// </summary>
        private double _camPhiSpeed = 0;

        /// <summary>
        /// Current automatic horizontal rotation speed
        /// </summary>
        private int _autoRotateSpeed = 0;

        /// <summary>
        /// Minimum allowed automatic horizontal rotation speed
        /// </summary>
        private const int _autoRotateSpeedMin = -10;

        /// <summary>
        /// Maximum allowed automatic horizontal rotation speed
        /// </summary>
        private const int _autoRotateSpeedMax = 10;

        /// <summary>
        /// Flag indicating whether the camera should move
        /// </summary>
        private bool _shouldMove = false;

        /// <summary>
        /// X coordinate of the mouse press
        /// </summary>
        private double _mouseX = 0;

        /// <summary>
        /// Y coordinate of the mouse press
        /// </summary>
        private double _mouseY = 0;

        /// <summary>
        /// Camera movement timer
        /// </summary>
        private DispatcherTimer _cameraMovementTimer = null;

        #endregion

        #region Constructor

        /// <summary>
        /// Class constructor
        /// </summary>
        public SphereViewer()
        {
            InitializeComponent();

            Reset();
        }

        #endregion

        #region Image loading

        /// <summary>
        /// Resets control to its defaults
        /// </summary>
        private void Reset()
        {
            _image = null;

            _sphereMesh = Tessellate(40, 20, 10);

            _brush = new ImageBrush()
            {
                TileMode = TileMode.Tile
            };

            _camTheta = 180;
            _camPhi = 90;
            _camThetaSpeed = 0;
            _camPhiSpeed = 0;
            _autoRotateSpeed = 0;

            _shouldMove = false;
            _mouseX = 0;
            _mouseY = 0;

            _cameraMovementTimer = new DispatcherTimer()
            {
                Interval = TimeSpan.FromMilliseconds(10)
            };

            _cameraMovementTimer.Tick += CameraMovementTimer_Tick;
            _cameraMovementTimer.Start();

            this.Cursor = Cursors.Arrow;

            modelVisual.Children.Clear();
            perspectiveCamera.FieldOfView = 100;
            perspectiveCamera.LookDirection = new Vector3D(0, -1, 0);
        }

        /// <summary>
        /// Loads and views equirectangular image in the sphere
        /// </summary>
        /// <param name="path">Path to the image file</param>
        public void LoadImage(string path)
        {
            Reset();

            if (string.IsNullOrEmpty(path))
                return;

            try
            {
                // var stopWatch = System.Diagnostics.Stopwatch.StartNew();

                // Load image as bitmap
                // Note: The same image is loaded in SphereViewer first (takes > 100 ms) and then in FlatViewer (takes 0 ms)
                _image = new BitmapImage();
                _image.BeginInit();
                _image.UriSource = new Uri(path);
                _image.CacheOption = BitmapCacheOption.OnLoad;
                _image.EndInit();

                // stopWatch.Stop();
                // System.Diagnostics.Trace.WriteLine("SphereViewer loading took " + stopWatch.ElapsedMilliseconds + "ms");
            }
            catch (Exception ex)
            {
                throw new ImageLoadingException(path, ex);
            }
            
            _brush.ImageSource = _image;

            ModelVisual3D sphereModel = new ModelVisual3D()
            {
                Content = new GeometryModel3D(_sphereMesh, new DiffuseMaterial(_brush))
            };

            modelVisual.Children.Clear();
            modelVisual.Children.Add(sphereModel);

            ApplyZoom(100);
        }

        #endregion

        #region Rotation

        public int GetAutoRotateSpeed()
        {
            return _autoRotateSpeed;
        }

        public void AutoRotateStop()
        {
            _autoRotateSpeed = 0;
            _shouldMove = false;
        }

        public bool CanAutoRotateLeft()
        {
            return (_autoRotateSpeed > _autoRotateSpeedMin);
        }

        public void AutoRotateLeft()
        {
            if (!CanAutoRotateLeft())
                return;

            _autoRotateSpeed -= 1;

            _shouldMove = (_autoRotateSpeed != 0);

            _camPhiSpeed = 0;
            _camThetaSpeed = _autoRotateSpeed;
        }

        public bool CanAutoRotateRight()
        {
            return (_autoRotateSpeed < _autoRotateSpeedMax);
        }

        public void AutoRotateRight()
        {
            if (!CanAutoRotateRight())
                return;

            _autoRotateSpeed += 1;

            _shouldMove = (_autoRotateSpeed != 0);

            _camPhiSpeed = 0;
            _camThetaSpeed = _autoRotateSpeed;
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

        #region Mouse interaction

        /// <summary>
        /// Move camera on timer tick
        /// </summary>
        /// <param name="sender">Who knows</param>
        /// <param name="e">Who knows</param>
        private void CameraMovementTimer_Tick(object sender, EventArgs e)
        {
            if (!_shouldMove)
                return;

            _camTheta += _camThetaSpeed / 50;
            _camPhi += _camPhiSpeed / 50;

            if (_camTheta < 0)
            {
                _camTheta += 360;
            }
            else if (_camTheta > 360)
            {
                _camTheta -= 360;
            }

            if (_camPhi < 0.01)
            {
                _camPhi = 0.01;
            }
            else if (_camPhi > 179.99)
            {
                _camPhi = 179.99;
            }

            perspectiveCamera.LookDirection = GetNormal(Deg2Rad(_camTheta), Deg2Rad(_camPhi));
        }

        /// <summary>
        /// Start moving the camera on mouse down
        /// </summary>
        /// <param name="sender">Who knows</param>
        /// <param name="e">Who knows</param>
        private void ViewPort_MouseDown(object sender, MouseButtonEventArgs e)
        {
            _shouldMove = true;

            _mouseX = Mouse.GetPosition(viewPort).X;
            _mouseY = Mouse.GetPosition(viewPort).Y;

            _camThetaSpeed = 0;
            _camPhiSpeed = 0;
            _autoRotateSpeed = 0;

            this.Cursor = Cursors.Hand;
        }

        /// <summary>
        /// Stop moving the camera on mouse up
        /// </summary>
        /// <param name="sender">Who knows</param>
        /// <param name="e">Who knows</param>
        private void ViewPort_MouseUp(object sender, MouseButtonEventArgs e)
        {
            _shouldMove = false;

            this.Cursor = Cursors.Arrow;
        }

        /// <summary>
        /// Set camera movement speed on mouse move
        /// </summary>
        /// <param name="sender">Who knows</param>
        /// <param name="e">Who knows</param>
        private void ViewPort_MouseMove(object sender, MouseEventArgs e)
        {
            if (!_shouldMove)
                return;

            if (e.LeftButton == MouseButtonState.Pressed)
            {
                _camThetaSpeed = (Mouse.GetPosition(viewPort).X - _mouseX) * -1;
                _camPhiSpeed = (Mouse.GetPosition(viewPort).Y - _mouseY) * -1;

                _camThetaSpeed = _camThetaSpeed / 8;
                _camPhiSpeed = _camPhiSpeed / 8;
            }

            if (e.MiddleButton == MouseButtonState.Pressed)
            {
                _camThetaSpeed = Mouse.GetPosition(viewPort).X - _mouseX;
                _camPhiSpeed = Mouse.GetPosition(viewPort).Y - _mouseY;

                _camThetaSpeed = _camThetaSpeed / 8;
                _camPhiSpeed = _camPhiSpeed / 8;
            }
        }

        /// <summary>
        /// Zoom on mouse wheel movement
        /// </summary>
        /// <param name="sender">Who knows</param>
        /// <param name="e">Who knows</param>
        private void ViewPort_MouseWheel(object sender, MouseWheelEventArgs e)
        {
            if (e.Delta >= 0)
            {
                ApplyZoom(zoomControl.ZoomValue + 5);
            }
            else
            {
                ApplyZoom(zoomControl.ZoomValue - 5);
            }
        }

        /// <summary>
        /// Zoom without mouse position
        /// </summary>
        /// <param name="zoom">Zoom value</param>
        private void ApplyZoom(double zoom)
        {
            // TODO - ApplyZoomAtPoint would be a nice addition

            zoomControl.ZoomValue = zoom; // Note: This also normalizes value

            if (zoomControl.ZoomValue == 100)
            {
                // Both ZOOM and FOV start at 100 (100 FOV = 100 ZOOM)
                perspectiveCamera.FieldOfView = zoomControl.ZoomValue;
            }
            else if (zoomControl.ZoomValue < 100)
            {
                // ZOOM goes down FOV goes up (125 FOV = 0 ZOOM)
                double fov = ((100 - zoomControl.ZoomValue) / 4) + 100;
                perspectiveCamera.FieldOfView = fov;
            }
            else if (zoomControl.ZoomValue > 100)
            {
                // ZOOM goes up FOV goes down (25 FOV = 400 ZOOM)
                double fov = 100 - ((zoomControl.ZoomValue - 100) / 4);
                perspectiveCamera.FieldOfView = fov;
            }
        }

        #endregion

        #region Zooming

        /// <summary>
        /// Zoom in from the outside
        /// </summary>
        public void ZoomIn()
        {
            ApplyZoom(zoomControl.ZoomValue + 5);
        }

        /// <summary>
        /// Zoom out from the outside
        /// </summary>
        public void ZoomOut()
        {
            ApplyZoom(zoomControl.ZoomValue - 5);
        }

        /// <summary>
        /// Reset zoom from the outside
        /// </summary>
        public void ZoomReset()
        {
            ApplyZoom(100);
        }

        #endregion

        #region Geometry utils

        /// <summary>
        /// Get normal vector for given angles
        /// </summary>
        /// <param name="theta">Theta angle</param>
        /// <param name="phi">Phi angle</param>
        /// <returns></returns>
        private static Vector3D GetNormal(double theta, double phi)
        {
            return ExternalCodes._3DTools.InteractiveSphere.GetNormal(theta, phi);
        }

        /// <summary>
        /// Convert degrees to radians
        /// </summary>
        /// <param name="degrees">Value in degrees</param>
        /// <returns>Value in radians</returns>
        private static double Deg2Rad(double degrees)
        {
            return ExternalCodes._3DTools.InteractiveSphere.DegToRad(degrees);
        }

        /// <summary>
        /// Create a tessellated sphere mesh
        /// </summary>
        /// <param name="thetaDivs">Theta divisions</param>
        /// <param name="phiDivs">Phi divisions</param>
        /// <param name="radius">Radius</param>
        /// <returns>Sphere mesh</returns>
        private static MeshGeometry3D Tessellate(int thetaDivs, int phiDivs, double radius)
        {
            return ExternalCodes._3DTools.InteractiveSphere.Tessellate(thetaDivs, phiDivs, radius);
        }

        #endregion
    }
}
