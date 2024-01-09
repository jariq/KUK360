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
using System.Collections.Generic;
using System.ComponentModel;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.Windows.Markup;

namespace KUK360.ExternalCodes.FontAwesome.WPF.Converters
{
    /// <summary>
    /// Converts a FontAwesomIcon to its description.
    /// </summary>
    public class DescriptionConverter
        : MarkupExtension, IValueConverter
    {
        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            return this;
        }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (!(value is FontAwesomeIcon)) return null;

            var icon = (FontAwesomeIcon) value;

            var memInfo = typeof(FontAwesomeIcon).GetMember(icon.ToString());
            var attributes = memInfo[0].GetCustomAttributes(typeof(DescriptionAttribute), false);

            if (attributes.Length == 0) return null; // alias

            return ((DescriptionAttribute)attributes[0]).Description;
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
