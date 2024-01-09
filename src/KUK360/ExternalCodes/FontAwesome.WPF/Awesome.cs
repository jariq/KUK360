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
    /// Provides attached properties to set FontAwesome icons on controls.
    /// </summary>
    public static class Awesome
    {
        /// <summary>
        /// Resource URI for FontAwesome.otf
        /// </summary>
        public static string Kuk360AwesomePackUri = @"pack://application:,,,/ExternalCodes/FontAwesome/FontAwesome.otf";

        /// <summary>
        /// FontAwesome FontFamily.
        /// </summary>
        private static readonly FontFamily FontAwesomeFontFamily = new FontFamily(new Uri(Kuk360AwesomePackUri), "./#FontAwesome");

        /// <summary>
        /// Identifies the FontAwesome.WPF.Awesome.Content attached dependency property.
        /// </summary>
        public static readonly DependencyProperty ContentProperty =
            DependencyProperty.RegisterAttached(
                "Content",
                typeof(FontAwesomeIcon),
                typeof(Awesome),
                new PropertyMetadata(DEFAULT_CONTENT, ContentChanged));

        /// <summary>
        /// Gets the content of a ContentControl, expressed as a FontAwesome icon.
        /// </summary>
        /// <param name="target">The ContentControl subject of the query</param>
        /// <returns>FontAwesome icon found as content</returns>
        public static FontAwesomeIcon GetContent(DependencyObject target)
        {
            return (FontAwesomeIcon)target.GetValue(ContentProperty);
        }
        
        /// <summary>
        /// Sets the content of a ContentControl expressed as a FontAwesome icon. This will cause the content to be redrawn.
        /// </summary>
        /// <param name="target">The ContentControl where to set the content</param>
        /// <param name="value">FontAwesome icon to set as content</param>
        public static void SetContent(DependencyObject target, FontAwesomeIcon value)
        {
            target.SetValue(ContentProperty, value);
        }

        private static void ContentChanged(DependencyObject sender, DependencyPropertyChangedEventArgs evt)
        {
            // If target is not a ContenControl just ignore: Awesome.Content property can only be set on a ContentControl element
            if (!(sender is ContentControl)) return;

            ContentControl target = (ContentControl) sender;

            // If value is not a FontAwesomeIcon just ignore: Awesome.Content property can only be set to a FontAwesomeIcon value
            if (!(evt.NewValue is FontAwesomeIcon)) return;

            FontAwesomeIcon symbolIcon = (FontAwesomeIcon)evt.NewValue;
            int symbolCode = (int)symbolIcon;
            char symbolChar = (char)symbolCode;

            target.FontFamily = FontAwesomeFontFamily;
            target.Content = symbolChar;
        }

        private const FontAwesomeIcon DEFAULT_CONTENT = FontAwesomeIcon.None;
    }
}
