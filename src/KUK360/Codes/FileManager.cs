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
using System.Collections.Generic;
using System.IO;
using System.Linq;

namespace KUK360.Codes
{
    internal class FileManager
    {
        public ImageFormatManager ImageFormatManager
        {
            get;
            protected set;
        }

        public string CurrentFile
        {
            get;
            protected set;
        }

        public string PreviousFile
        {
            get;
            protected set;
        }

        public string NextFile
        {
            get;
            protected set;
        }

        public FileManager()
        {
            this.ImageFormatManager = new ImageFormatManager();
        }

        public void SetCurrentFile(string path)
        {
            if (string.IsNullOrEmpty(path))
            {
                this.CurrentFile = null;
                this.PreviousFile = null;
                this.NextFile = null;
                return;
            }

            if (!File.Exists(path))
                throw new FileNotFoundException($"File {path} does not exist");

            if (!IsExtSupported(Path.GetExtension(path)))
                throw new Exception($"File {path} is of unsupported type");

            // Get sorted list of all supported files from the same directory
            List<string> files = Directory.EnumerateFiles(Path.GetDirectoryName(path))
                .Where(file => this.ImageFormatManager.SupportedExtensions.Contains(Path.GetExtension(file).ToLower()))
                .ToList();

            files.Sort();

            // Determine path of previous, current and next file
            if (files.Count < 1)
            {
                this.CurrentFile = null;
                this.PreviousFile = null;
                this.NextFile = null;
            }
            else if (files.Count == 1)
            {
                this.CurrentFile = files[0];
                this.PreviousFile = null;
                this.NextFile = null;
            }
            else
            {
                for (int i = 0; i < files.Count; i++)
                {
                    if (files[i] == path)
                    {
                        this.CurrentFile = files[i];
                        this.PreviousFile = (i != 0) ? files[i - 1] : files[files.Count - 1];
                        this.NextFile = (i != files.Count - 1) ? files[i + 1] : files[0];
                    }
                }
            }
        }

        public bool IsExtSupported(string ext)
        {
            string lowerExt = ext.ToLower();
            return this.ImageFormatManager.SupportedExtensions.Contains(lowerExt);
        }
    }
}
