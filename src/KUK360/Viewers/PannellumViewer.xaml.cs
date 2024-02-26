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
using System.Reflection;
using System.Threading.Tasks;
using System.Web;
using System.Windows;
using System.Windows.Controls;
using KUK360.Codes;
using Microsoft.Web.WebView2.Core;

namespace KUK360.Viewers
{
    /// <summary>
    /// Interaction logic for PannellumViewer.xaml
    /// </summary>
    public partial class PannellumViewer : UserControl, IThreeSixtyImageViewer
    {
        #region Private fields

        private bool _initialized = false;

        private string _pannellumUrl1 = null;

        private string _pannellumUrl2 = null;

        private string _pannellumConfig = null;

        #endregion

        #region Constructor

        public PannellumViewer()
        {
            InitializeComponent();
        }

        #endregion

        #region Visibility

        public bool IsShown()
        {
            return this.Visibility == Visibility.Visible;
        }

        public void Show()
        {
            this.Visibility = Visibility.Visible;
        }

        public void Hide()
        {
            this.Visibility = Visibility.Collapsed;
        }

        #endregion

        #region Image loading

        private async Task Reset()
        {
            await InitializeWebView2();

            // TODO - Resets control to its defaults
        }

        private async Task InitializeWebView2()
        {
            if (!_initialized)
            {
                await webView2.EnsureCoreWebView2Async();

                string appDir = System.IO.Path.GetDirectoryName(Assembly.GetExecutingAssembly().CodeBase).Remove(0, 6); // remove file:\\
                string pannellumDir = System.IO.Path.Combine(appDir, "ExternalCodes", "Pannellum");

                webView2.CoreWebView2.SetVirtualHostNameToFolderMapping("www.kuk360.invalid", pannellumDir, CoreWebView2HostResourceAccessKind.Allow);

                _pannellumUrl1 = "http://www.kuk360.invalid/" + "pannellum1.htm";
                _pannellumUrl2 = "http://www.kuk360.invalid/" + "pannellum2.htm";
                _pannellumConfig = "http://www.kuk360.invalid/" + "pannellum.json";

                // webView2.CoreWebView2.OpenDevToolsWindow();

                _initialized = true;
            }
        }

        public async void LoadImage(string path)
        {
            await Reset();

            if (string.IsNullOrEmpty(path))
                return;

            string imgDir = System.IO.Path.GetDirectoryName(path);
            string imgFile = System.IO.Path.GetFileName(path);
            string imgFileEncoded = HttpUtility.UrlEncode(imgFile);
            string imgUrl = "http://img.kuk360.invalid/" + imgFileEncoded;

            webView2.CoreWebView2.ClearVirtualHostNameToFolderMapping("img.kuk360.invalid");
            webView2.CoreWebView2.SetVirtualHostNameToFolderMapping("img.kuk360.invalid", imgDir, CoreWebView2HostResourceAccessKind.Allow);

            string url = (webView2.Source.ToString().StartsWith(_pannellumUrl1)) ? _pannellumUrl2 : _pannellumUrl1;
            url += $"#config={_pannellumConfig}&panorama={imgUrl}";

            webView2.Source = new Uri(url);
        }

        #endregion

        #region Rotation

        public int GetAutoRotateSpeed()
        {
            throw new NotImplementedException();
        }

        public void AutoRotateStop()
        {
            // throw new NotImplementedException();
        }

        public bool CanAutoRotateLeft()
        {
            return false;
        }

        public void AutoRotateLeft()
        {
            throw new NotImplementedException();
        }

        public bool CanAutoRotateRight()
        {
            return false;
        }

        public void AutoRotateRight()
        {
            throw new NotImplementedException();
        }

        #endregion

        #region ZoomControl

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

        public void ShowZoomControl()
        {
            this.zoomControl.Visibility = Visibility.Visible;
        }

        public void HideZoomControl()
        {
            this.zoomControl.Visibility = Visibility.Hidden;
        }

        private void zoomControl_ZoomValueChanged(double zoomValue)
        {
            throw new NotImplementedException();
        }

        #endregion

        #region Zooming

        public void ZoomIn()
        {
            throw new NotImplementedException();
        }

        public void ZoomOut()
        {
            throw new NotImplementedException();
        }

        public void ZoomReset()
        {
            throw new NotImplementedException();
        }

        #endregion
    }
}
