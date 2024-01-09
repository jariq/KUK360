/* 
 * The code in this file was copied from an external project
 * and then slightly modified to suit our needs.
 * 
 * Original project: FontAwesome.WPF 4.7.0.9
 * Original repository: https://github.com/charri/Font-Awesome-WPF/tree/v4.7
 * Original license: MIT
 * 
 * The MIT License (MIT)
 * 
 * Copyright (c) 2014-2016 charri
 * 
 * Permission is hereby granted, free of charge, to any person obtaining 
 * a copy of this software and associated documentation files (the "Software"), 
 * to deal in the Software without restriction, including without limitation 
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, 
 * and/or sell copies of the Software, and to permit persons to whom 
 * the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included 
 * in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

using System;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;

namespace KUK360.ExternalCodes.FontAwesome.WPF
{
    /// <summary>
    /// Provides a lightweight control for displaying a FontAwesome icon as text.
    /// </summary>
    public class FontAwesome
        : TextBlock, ISpinable, IRotatable, IFlippable
    {
        /// <summary>
        /// FontAwesome FontFamily.
        /// </summary>
        private static readonly FontFamily FontAwesomeFontFamily = new FontFamily(new Uri(Awesome.Kuk360AwesomePackUri), "./#FontAwesome");
        /// <summary>
        /// Identifies the FontAwesome.WPF.FontAwesome.Icon dependency property.
        /// </summary>
        public static readonly DependencyProperty IconProperty =
            DependencyProperty.Register("Icon", typeof(FontAwesomeIcon), typeof(FontAwesome), new PropertyMetadata(FontAwesomeIcon.None, OnIconPropertyChanged));
        /// <summary>
        /// Identifies the FontAwesome.WPF.FontAwesome.Spin dependency property.
        /// </summary>
        public static readonly DependencyProperty SpinProperty =
            DependencyProperty.Register("Spin", typeof(bool), typeof(FontAwesome), new PropertyMetadata(false, OnSpinPropertyChanged, SpinCoerceValue));
        /// <summary>
        /// Identifies the FontAwesome.WPF.FontAwesome.Spin dependency property.
        /// </summary>
        public static readonly DependencyProperty SpinDurationProperty =
            DependencyProperty.Register("SpinDuration", typeof(double), typeof(FontAwesome), new PropertyMetadata(1d, SpinDurationChanged, SpinDurationCoerceValue));
        /// <summary>
        /// Identifies the FontAwesome.WPF.FontAwesome.Rotation dependency property.
        /// </summary>
        public static readonly DependencyProperty RotationProperty =
            DependencyProperty.Register("Rotation", typeof(double), typeof(FontAwesome), new PropertyMetadata(0d, RotationChanged, RotationCoerceValue));
        /// <summary>
        /// Identifies the FontAwesome.WPF.FontAwesome.FlipOrientation dependency property.
        /// </summary>
        public static readonly DependencyProperty FlipOrientationProperty =
            DependencyProperty.Register("FlipOrientation", typeof(FlipOrientation), typeof(FontAwesome), new PropertyMetadata(FlipOrientation.Normal, FlipOrientationChanged));

        static FontAwesome()
        {
            OpacityProperty.OverrideMetadata(typeof(FontAwesome), new UIPropertyMetadata(1.0, OpacityChanged));
        }

        private static void OpacityChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            d.CoerceValue(SpinProperty);
        }

        /// <summary>
        /// Gets or sets the FontAwesome icon. Changing this property will cause the icon to be redrawn.
        /// </summary>
        public FontAwesomeIcon Icon
        {
            get { return (FontAwesomeIcon)GetValue(IconProperty); }
            set { SetValue(IconProperty, value); }
        }
        /// <summary>
        /// Gets or sets the current spin (angle) animation of the icon.
        /// </summary>
        public bool Spin
        {
            get { return (bool)GetValue(SpinProperty); }
            set { SetValue(SpinProperty, value); }
        }

        private static void OnIconPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            d.SetValue(TextOptions.TextRenderingModeProperty, TextRenderingMode.ClearType);
            d.SetValue(FontFamilyProperty, FontAwesomeFontFamily);
            d.SetValue(TextAlignmentProperty, TextAlignment.Center);
            d.SetValue(TextProperty, char.ConvertFromUtf32((int)e.NewValue));
        }

        private static void OnSpinPropertyChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var fontAwesome = d as FontAwesome;

            if (fontAwesome == null) return;

            if ((bool)e.NewValue)
                fontAwesome.BeginSpin();
            else
            {
                fontAwesome.StopSpin();
                fontAwesome.SetRotation();
            }
        }

        private static object SpinCoerceValue(DependencyObject d, object basevalue)
        {
            var fontAwesome = (FontAwesome)d;
            if (fontAwesome.Opacity == 0.0 || fontAwesome.SpinDuration == 0.0)
                return false;

            return basevalue;
        }

        /// <summary>
        /// Gets or sets the duration of the spinning animation (in seconds). This will stop and start the spin animation.
        /// </summary>
        public double SpinDuration
        {
            get { return (double)GetValue(SpinDurationProperty); }
            set { SetValue(SpinDurationProperty, value); }
        }

        private static void SpinDurationChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var fontAwesome = d as FontAwesome;

            if (null == fontAwesome || !fontAwesome.Spin || !(e.NewValue is double) || e.NewValue.Equals(e.OldValue)) return;

            fontAwesome.StopSpin();
            fontAwesome.BeginSpin();
        }

        private static object SpinDurationCoerceValue(DependencyObject d, object value)
        {
            double val = (double)value;
            return val < 0 ? 0d : value;
        }

        /// <summary>
        /// Gets or sets the current rotation (angle).
        /// </summary>
        public double Rotation
        {
            get { return (double)GetValue(RotationProperty); }
            set { SetValue(RotationProperty, value); }
        }

        private static void RotationChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var fontAwesome = d as FontAwesome;

            if (null == fontAwesome || fontAwesome.Spin || !(e.NewValue is double) || e.NewValue.Equals(e.OldValue)) return;

            fontAwesome.SetRotation();
        }

        private static object RotationCoerceValue(DependencyObject d, object value)
        {
            double val = (double)value;
            return val < 0 ? 0d : (val > 360 ? 360d : value);
        }

        /// <summary>
        /// Gets or sets the current orientation (horizontal, vertical).
        /// </summary>
        public FlipOrientation FlipOrientation
        {
            get { return (FlipOrientation)GetValue(FlipOrientationProperty); }
            set { SetValue(FlipOrientationProperty, value); }
        }

        private static void FlipOrientationChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
        {
            var fontAwesome = d as FontAwesome;

            if (null == fontAwesome || !(e.NewValue is FlipOrientation) || e.NewValue.Equals(e.OldValue)) return;

            fontAwesome.SetFlipOrientation();
        }
    }
}
