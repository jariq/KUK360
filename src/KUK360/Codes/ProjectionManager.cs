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

using System.ComponentModel;
using System.IO;
using System.Text;

namespace KUK360.Codes
{
    public class ProjectionManager : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        private bool _autoDetectionEnabled = true;

        public bool AutoDetectionEnabled
        {
            get
            {
                return _autoDetectionEnabled;
            }
            set
            {
                if (_autoDetectionEnabled == value)
                    return;

                _autoDetectionEnabled = value;

                PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(nameof(AutoDetectionEnabled)));
            }
        }

        public ProjectionType Projection
        {
            get;
            set;
        }

        public ProjectionManager()
        {
            _autoDetectionEnabled = true;
            Projection = ProjectionType.Flat;
        }

        public ProjectionType DetermineImageProjection(string path)
        {
            if (AutoDetectionEnabled)
                Projection = IsEquirectangularImage(path) ? ProjectionType.Sphere : ProjectionType.Flat;

            return Projection;
        }

        private bool IsEquirectangularImage(string path)
        {
            byte[] pattern = Encoding.ASCII.GetBytes("equirectangular");
            int patternLength = pattern.Length;

            byte[] content = new byte[1048576];
            int contentLength = content.Length;

            using (var stream = new FileStream(path, FileMode.Open, FileAccess.Read))
            {
                contentLength = stream.Read(content, 0, contentLength);
            }

            for (int i = 0; i < contentLength - patternLength; i++)
            {
                if (content[i] == pattern[0])
                {
                    bool patternFullyMatches = true;

                    for (int j = 1; j < patternLength; j++)
                    {
                        if (content[i + j] != pattern[j])
                        {
                            patternFullyMatches = false;
                            break;
                        }
                    }

                    if (patternFullyMatches)
                        return true;
                }
            }

            return false;
        }
    }
}
