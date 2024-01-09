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
using System.Diagnostics;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows.Data;
using System.Windows.Markup;

namespace KUK360.ExternalCodes.FontAwesome.WPF.Converters
{
    /// <summary>
    /// Converts the CSS class name to a FontAwesomIcon and vice-versa.
    /// </summary>
    public class CssClassNameConverter
        : MarkupExtension, IValueConverter
    {
        private static readonly IDictionary<string, FontAwesomeIcon> ClassNameLookup = new Dictionary<string, FontAwesomeIcon>();
        private static readonly IDictionary<FontAwesomeIcon, string> IconLookup = new Dictionary<FontAwesomeIcon, string>();

        static CssClassNameConverter()
        {
            foreach (var value in Enum.GetValues(typeof(FontAwesomeIcon)))
            {
                var memInfo = typeof(FontAwesomeIcon).GetMember(value.ToString());
                var attributes = memInfo[0].GetCustomAttributes(typeof(IconIdAttribute), false);

                if (attributes.Length == 0) continue; // alias

                var id = ((IconIdAttribute)attributes[0]).Id;

                if (ClassNameLookup.ContainsKey(id)) continue;

                ClassNameLookup.Add(id, (FontAwesomeIcon)value);
                IconLookup.Add((FontAwesomeIcon)value, id);
            }
        }

        /// <summary>
        /// Gets or sets the mode of the converter
        /// </summary>
        public CssClassConverterMode Mode { get; set; }

        private static FontAwesomeIcon FromStringToIcon(object value)
        {
            var icon = value as string;

            if (string.IsNullOrEmpty(icon)) return FontAwesomeIcon.None;

            FontAwesomeIcon rValue;

            if (!ClassNameLookup.TryGetValue(icon, out rValue))
            {
                rValue = FontAwesomeIcon.None;
            }

            return rValue;
        }

        private static string FromIconToString(object value)
        {
            if (!(value is FontAwesomeIcon)) return null;

            string rValue = null;

            IconLookup.TryGetValue((FontAwesomeIcon) value, out rValue);
            
            return rValue;
        }

        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (Mode == CssClassConverterMode.FromStringToIcon)
                return FromStringToIcon(value);
            
            return FromIconToString(value);
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (Mode == CssClassConverterMode.FromStringToIcon)
                return FromIconToString(value);

            return FromStringToIcon(value);
        }

        public override object ProvideValue(IServiceProvider serviceProvider)
        {
            return this;
        }
    }

    /// <summary>
    /// Defines the CssClassNameConverter mode. 
    /// </summary>
    public enum CssClassConverterMode
    {
        /// <summary>
        /// Default mode. Expects a string and converts to a FontAwesomeIcon.
        /// </summary>
        FromStringToIcon = 0,
        /// <summary>
        /// Expects a FontAwesomeIcon and converts it to a string.
        /// </summary>
        FromIconToString
    }
}
