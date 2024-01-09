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

using System.IO;
using System.Reflection;
using System.Windows;
using System.Windows.Documents;

namespace KUK360.Windows
{
    public partial class AboutWindow : Window
    {
        public AboutWindow()
        {
            InitializeComponent();

            VersionedTitle.Inlines.Clear();
            VersionedTitle.Inlines.Add("KUK360 " + GetVersion());

            LoadLicenseFile();
        }

        private string GetVersion()
        {
            object[] attributes = Assembly.GetExecutingAssembly().GetCustomAttributes(typeof(AssemblyFileVersionAttribute), false);
            if (attributes.Length == 0)
            {
                return "";
            }
            return "v" + ((AssemblyFileVersionAttribute)attributes[0]).Version;
        }

        private void LoadLicenseFile()
        {
            if (File.Exists("LICENSE.rtf"))
            {
                TextRange range = new TextRange(LicenseBox.Document.ContentStart, LicenseBox.Document.ContentEnd);
                using (FileStream fs = new FileStream("LICENSE.rtf", FileMode.Open, FileAccess.Read))
                    range.Load(fs, DataFormats.Rtf);
            }
        }
    }
}
