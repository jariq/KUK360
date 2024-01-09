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

using Microsoft.Win32;
using System;
using System.Collections.Generic;

namespace KUK360.Codes
{
    internal class ImageFormatManager
    {
        private HashSet<string> _supportedExtensions = new HashSet<string>();

        public HashSet<string> SupportedExtensions
        {
            get
            {
                return _supportedExtensions;
            }
        }

        private string _openFileDialogFilter = null;

        public string OpenFileDialogFilter
        {
            get
            {
                return _openFileDialogFilter;
            }
        }

        public ImageFormatManager()
        {
            // Add formats natively supported by WPF
            _supportedExtensions.Add(".jpg");
            _supportedExtensions.Add(".jpeg");
            _supportedExtensions.Add(".png");
            _supportedExtensions.Add(".tiff");
            _supportedExtensions.Add(".tif");
            _supportedExtensions.Add(".bmp");
            _supportedExtensions.Add(".gif");

            // Add formats supported by WIC decoders
            // Note: On my computer opening .CR2 file from Canon 80d spent over 30 seconds in System.Windows.Media.Imaging.BitmapImage::EndInit
            // ReadAdditionalSupportedExtensions();

            CreateOpenFileDialogFilter();
        }

        private void ReadAdditionalSupportedExtensions()
        {
            try
            {
                string baseKeyPath = (Environment.Is64BitOperatingSystem && !Environment.Is64BitProcess) ? @"Wow6432Node\CLSID" : "CLSID";
                using (RegistryKey baseKey = Registry.ClassesRoot.OpenSubKey(baseKeyPath, false))
                {
                    if (baseKey != null)
                    {
                        // Read Windows Imaging Component (WIC) decoders
                        using (RegistryKey decodersKey = baseKey.OpenSubKey(@"{7ED96837-96F0-4812-B211-F13C24117ED3}\instance", false))
                        {
                            if (decodersKey != null)
                            {
                                // Read the GUIDs of the registered decoders
                                string[] decoderGuids = decodersKey.GetSubKeyNames();
                                foreach (string decoderGuid in decoderGuids)
                                {
                                    // Read the properties of the single registered decoder
                                    using (RegistryKey decoderKey = baseKey.OpenSubKey(decoderGuid))
                                    {
                                        if (decoderKey != null)
                                        {
                                            string decoderFileExtensions = Convert.ToString(decoderKey.GetValue("FileExtensions", ""));
                                            string[] decoderExtensions = decoderFileExtensions.Split(',');

                                            foreach (string decoderExtension in decoderExtensions)
                                            {
                                                if (!_supportedExtensions.Contains(decoderExtension.ToLower()))
                                                    _supportedExtensions.Add(decoderExtension.ToLower());
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            catch
            {
                // TODO - Log exception or something ???
            }
        }

        private void CreateOpenFileDialogFilter()
        {
            string allImages = "All images|";

            foreach (string ext in _supportedExtensions)
            {
                _openFileDialogFilter += ext.Remove(0, 1).ToUpper()  + " images|*" + ext + "|";
                allImages += "*" + ext + ";";
            }

            _openFileDialogFilter += allImages + "|";

            _openFileDialogFilter += "All files|*.*";
        }
    }
}
