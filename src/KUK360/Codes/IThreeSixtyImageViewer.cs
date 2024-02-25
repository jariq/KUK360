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

namespace KUK360.Codes
{
    internal interface IThreeSixtyImageViewer
    {
        #region Visibility

        bool IsShown();

        void Show();

        void Hide();

        #endregion

        #region Image loading

        void LoadImage(string path);

        #endregion

        #region Rotation

        int GetAutoRotateSpeed();

        void AutoRotateStop();

        bool CanAutoRotateLeft();

        void AutoRotateLeft();

        bool CanAutoRotateRight();

        void AutoRotateRight();

        #endregion

        #region ZoomControl

        void ToggleZoomControl();

        void ShowZoomControl();

        void HideZoomControl();

        #endregion

        #region Zooming

        void ZoomIn();

        void ZoomOut();

        void ZoomReset();

        #endregion
    }
}