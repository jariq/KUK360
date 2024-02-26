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
using System.Collections.Specialized;
using System.Diagnostics;
using System.IO;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;
using KUK360.Codes;
using Microsoft.VisualBasic.FileIO;
using Microsoft.Win32;

namespace KUK360.Windows
{
    public partial class MainWindow : Window
    {
        internal static Window LastInstance
        {
            get;
            private set;
        }

        internal IThreeSixtyImageViewer ThreeSixtyViewer
        {
            get
            {
                //return sphereMeshViewer;
                return pannellumViewer;
            }
        }

        internal IStandardImageViewer StandardViewer
        {
            get
            {
                return flatViewer;
            }
        }

        private FullScreenManager _fullScreenManager = null;

        private FileManager _fileManager = new FileManager();

        private ProjectionManager _projectionManager = new ProjectionManager();

        private string _draggedFile = null;

        public MainWindow()
        {
            LastInstance = this;

            InitializeComponent();

            Application.Current.DispatcherUnhandledException += GlobalExceptionHandler;

            _fullScreenManager = new FullScreenManager(this);

            // Set data contexts
            menuItemAutomaticDetection.DataContext = _projectionManager;
            ((ContextMenu)this.Resources["ViewerContexMenu"]).DataContext = _projectionManager;

            // Attach to this event so bubble can be show when "AutoDetectionEnabled" property is changed via context menu and application menu
            _projectionManager.PropertyChanged += ProjectionManager_PropertyChanged;

            // Set focus or else context menu won't be receiving events from commands
            FocusManager.SetFocusedElement(this, this);

            ViewFile(null);

            if (Environment.GetCommandLineArgs().Length > 1)
            {
                string argPath = Environment.GetCommandLineArgs()[1];
                if (!string.IsNullOrEmpty(argPath) && File.Exists(argPath))
                {
                    if (_fileManager.IsExtSupported(Path.GetExtension(argPath)))
                    {
                        ViewFile(argPath);
                    }
                }
            }
        }

        private void GlobalExceptionHandler(object sender, System.Windows.Threading.DispatcherUnhandledExceptionEventArgs e)
        {
            e.Handled = true;
            HandleException(e.Exception);
        }

        private void HandleException(Exception ex)
        {
            string message = string.Empty;
            message += "Application encountered unexpected error." + Environment.NewLine + Environment.NewLine;
            message += "Error description: " + (ex.Message ?? "Description is not available") + Environment.NewLine + Environment.NewLine;
            message += "Application will now close.";

            MessageBoxResult result = MessageBox.Show(this, message, "KUK360", MessageBoxButton.OKCancel, MessageBoxImage.Error);
            if (result == MessageBoxResult.OK)
            {
                Application.Current.Shutdown();
            }
        }

        private void DoEvents()
        {
            // Note: This should process all windows messages currently in the message queue
            Application.Current.Dispatcher.Invoke(System.Windows.Threading.DispatcherPriority.Background, new Action(delegate { }));
        }

        private void ViewFile(string path)
        {
            if (string.IsNullOrEmpty(path))
            {
                _fileManager.SetCurrentFile(path);

                ThreeSixtyViewer.LoadImage(null);
                ThreeSixtyViewer.HideZoomControl();
                HideThreeSixtyViewer();

                StandardViewer.LoadImage(null, true);
                StandardViewer.HideZoomControl();
                HideStandardViewer();

                DisplayOpenButtonInTextViewer();

                this.Title = $"KUK360";
            }
            else
            {
                try
                {
                    bubbleViewer.ShowPermanentBubble("Loading...");
                    DoEvents();

                    _fileManager.SetCurrentFile(path);

                    try
                    {
                        ProjectionType projection = _projectionManager.DetermineImageProjection(path);
                        if (projection == ProjectionType.ThreeSixty)
                        {

                            ThreeSixtyViewer.LoadImage(path);
                            DisplayImageInThreeSixtyViewer();
                            StandardViewer.LoadImage(path, true);
                        }
                        else
                        {
                            StandardViewer.LoadImage(path, true);
                            DisplayImageInStandardViewer();
                            ThreeSixtyViewer.LoadImage(path);
                        }
                    }
                    catch (ImageLoadingException)
                    {
                        ThreeSixtyViewer.LoadImage(null);
                        ThreeSixtyViewer.HideZoomControl();
                        HideThreeSixtyViewer();

                        StandardViewer.LoadImage(null, true);
                        StandardViewer.HideZoomControl();
                        HideStandardViewer();

                        bubbleViewer.HideBubble();
                        DisplayStaticTextInTextViewer("Unable to load image");
                    }

                    this.Title = $"KUK360 - {path}";
                }
                catch
                {
                    // Reset window to initial view and rethrow exception
                    ViewFile(null);
                    throw;
                }
            }
        }

        private bool IsImageDisplayed()
        {
            return (!string.IsNullOrEmpty(_fileManager.CurrentFile) && (IsThreeSixtyViewerDisplayed() || IsStandardViewerDisplayed()));
        }

        #region ThreeSixtyViewer

        private bool IsThreeSixtyViewerDisplayed()
        {
            return ThreeSixtyViewer.IsShown();
        }

        private void DisplayImageInThreeSixtyViewer()
        {
            KUK360.ExternalCodes.FontAwesome.WPF.Awesome.SetContent(btnProjectionSwitch, KUK360.ExternalCodes.FontAwesome.WPF.FontAwesomeIcon.Image);

            HideTextViewer();
            HideStandardViewer();
            ShowThreeSixtyViewer();

            bubbleViewer.ShowTemporaryBubble("360° projection");
        }

        private void ShowThreeSixtyViewer()
        {
            ThreeSixtyViewer.Show();
        }

        private void HideThreeSixtyViewer()
        {
            ThreeSixtyViewer.Hide();
        }

        #endregion

        #region StandardViewer

        private bool IsStandardViewerDisplayed()
        {
            return StandardViewer.IsShown();
        }

        private void DisplayImageInStandardViewer()
        {
            KUK360.ExternalCodes.FontAwesome.WPF.Awesome.SetContent(btnProjectionSwitch, KUK360.ExternalCodes.FontAwesome.WPF.FontAwesomeIcon.Globe);

            HideTextViewer();
            ShowStandardViewer();
            HideThreeSixtyViewer();

            bubbleViewer.ShowTemporaryBubble("Standard projection");
        }

        private void ShowStandardViewer()
        {
            StandardViewer.Show();
        }

        private void HideStandardViewer()
        {
            StandardViewer.Hide();
        }

        #endregion

        #region TextViewer

        private void DisplayOpenButtonInTextViewer()
        {
            ShowTextViewer();
            textViewer.ShowOpenButton();
        }

        private void DisplayStaticTextInTextViewer(string text)
        {
            ShowTextViewer();
            textViewer.ShowStaticText(text);
        }

        private void ShowTextViewer()
        {
            textViewer.Visibility = Visibility.Visible;
        }

        private void HideTextViewer()
        {
            textViewer.Visibility = Visibility.Collapsed;
        }

        #endregion

        #region Drag and drop

        private void GridMain_Drop(object sender, DragEventArgs e)
        {
            if (_draggedFile != null)
            {
                try
                {
                    ViewFile(_draggedFile);
                }
                catch (Exception ex)
                {
                    // Note: Drag-and-drop in WPF is handled through the standard unmanaged OLE drag-drop mechanism 
                    //       and because of that GlobalExceptionHandler won't catch exceptions generated in this event handler.
                    HandleException(ex);
                }
            }
        }

        private void GridMain_DragEnter(object sender, DragEventArgs e)
        {
            _draggedFile = null;

            if (e.Data.GetDataPresent(DataFormats.FileDrop))
            {
                string[] files = (string[])e.Data.GetData(DataFormats.FileDrop);
                if (files != null && files.Length == 1)
                {
                    string file = files[0];
                    if (File.Exists(file) && _fileManager.IsExtSupported(Path.GetExtension(file)))
                    {
                        _draggedFile = file;
                    }
                }
            }

            e.Handled = true;
        }

        private void GridMain_DragOver(object sender, DragEventArgs e)
        {
            e.Effects = (_draggedFile == null) ? DragDropEffects.None : DragDropEffects.Copy;
            e.Handled = true;
        }

        #endregion

        #region CmdFileOpen

        public static RoutedCommand CmdFileOpen = new RoutedCommand();

        private void CmdFileOpenCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdFileOpenExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Title = "Select image to be displayed";
            openFileDialog.Filter = _fileManager.ImageFormatManager.OpenFileDialogFilter;
            openFileDialog.FilterIndex = _fileManager.ImageFormatManager.SupportedExtensions.Count + 1;

            if (IsImageDisplayed())
                openFileDialog.InitialDirectory = Path.GetDirectoryName(_fileManager.CurrentFile);

            if (openFileDialog.ShowDialog() == true)
            {
                string selectedFile = openFileDialog.FileName;
                ViewFile(selectedFile);
            }
        }

        #endregion

        #region CmdFilePrevious

        public static RoutedCommand CmdFilePrevious = new RoutedCommand();

        private void CmdFilePreviousCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !string.IsNullOrEmpty(_fileManager.PreviousFile);
        }

        private void CmdFilePreviousExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            ViewFile(_fileManager.PreviousFile);
        }

        #endregion

        #region CmdFileNext

        public static RoutedCommand CmdFileNext = new RoutedCommand();

        private void CmdFileNextCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !string.IsNullOrEmpty(_fileManager.NextFile);
        }

        private void CmdFileNextExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            ViewFile(_fileManager.NextFile);
        }

        #endregion

        #region CmdFileClose

        public static RoutedCommand CmdFileClose = new RoutedCommand();

        private void CmdFileCloseCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = !string.IsNullOrEmpty(_fileManager.NextFile);
        }

        private void CmdFileCloseExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            ViewFile(null);
        }

        #endregion

        #region CmdFileOpenLocation

        public static RoutedCommand CmdFileOpenLocation = new RoutedCommand();

        private void CmdFileOpenLocationCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdFileOpenLocationExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            Process.Start("explorer.exe", $"/select,\"{_fileManager.CurrentFile}\"");
        }

        #endregion

        #region CmdFileExit

        public static RoutedCommand CmdFileExit = new RoutedCommand();

        private void CmdFileExitCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdFileExitExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            Application.Current.Shutdown();
        }

        #endregion

        #region CmdEditCopy

        public static RoutedCommand CmdEditCopy = new RoutedCommand();

        private void CmdEditCopyCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdEditCopyExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            StringCollection paths = new StringCollection();
            paths.Add(_fileManager.CurrentFile);

            Clipboard.Clear();
            Clipboard.SetFileDropList(paths);
        }

        #endregion

        #region CmdEditDelete

        public static RoutedCommand CmdEditDelete = new RoutedCommand();

        private void CmdEditDeleteCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdEditDeleteExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            MessageBoxResult result = MessageBox.Show(this, $"Do you really want to move displayed image {Path.GetFileName(_fileManager.CurrentFile)} to the recycle bin?", "KUK360", MessageBoxButton.YesNo, MessageBoxImage.Question);
            if (result == MessageBoxResult.Yes)
            {
                FileSystem.DeleteFile(_fileManager.CurrentFile, UIOption.OnlyErrorDialogs, RecycleOption.SendToRecycleBin);
                ViewFile(_fileManager.NextFile);
            }
        }

        #endregion

        #region CmdProjectionSwitch

        public static RoutedCommand CmdProjectionSwitch = new RoutedCommand();

        private void CmdProjectionSwitchCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdProjectionSwitchExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsThreeSixtyViewerDisplayed())
            {
                _projectionManager.Projection = ProjectionType.Standard;
                DisplayImageInStandardViewer();

                ThreeSixtyViewer.AutoRotateStop();
            }
            else
            {
                _projectionManager.Projection = ProjectionType.ThreeSixty;
                DisplayImageInThreeSixtyViewer();
            }
        }

        #endregion

        #region CmdProjectionAutomatic

        public static RoutedCommand CmdProjectionAutomatic = new RoutedCommand();

        private void CmdProjectionAutomaticCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdProjectionAutomaticExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            _projectionManager.AutoDetectionEnabled = !_projectionManager.AutoDetectionEnabled;

            bubbleViewer.ShowTemporaryBubble((_projectionManager.AutoDetectionEnabled ? "Enabled" : "Disabled") + " projection detection");
        }

        private void ProjectionManager_PropertyChanged(object sender, System.ComponentModel.PropertyChangedEventArgs e)
        {
            if (e.PropertyName == "AutoDetectionEnabled")
            {
                bubbleViewer.ShowTemporaryBubble((_projectionManager.AutoDetectionEnabled ? "Enabled" : "Disabled") + " projection detection");
            }
        }

        #endregion

        #region CmdZoomIn

        public static RoutedCommand CmdZoomIn = new RoutedCommand();

        private void CmdZoomInCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdZoomInExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsStandardViewerDisplayed())
            {
                StandardViewer.ZoomIn();
            }
            else
            {
                ThreeSixtyViewer.ZoomIn();
            }
        }

        #endregion

        #region CmdZoomOut

        public static RoutedCommand CmdZoomOut = new RoutedCommand();

        private void CmdZoomOutCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdZoomOutExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsStandardViewerDisplayed())
            {
                StandardViewer.ZoomOut();
            }
            else
            {
                ThreeSixtyViewer.ZoomOut();
            }
        }

        #endregion

        #region CmdZoom100

        public static RoutedCommand CmdZoom100 = new RoutedCommand();

        private void CmdZoom100CanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdZoom100Executed(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsStandardViewerDisplayed())
            {
                StandardViewer.ZoomReset();
            }
            else
            {
                ThreeSixtyViewer.ZoomReset();
            }
        }

        #endregion

        #region CmdZoomShow

        public static RoutedCommand CmdZoomShow = new RoutedCommand();

        private void CmdZoomShowCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdZoomShowExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            ThreeSixtyViewer.ToggleZoomControl();
            StandardViewer.ToggleZoomControl();
        }

        #endregion

        #region CmdRotateLeft

        public static RoutedCommand CmdRotateLeft = new RoutedCommand();

        private void CmdRotateLeftCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            if (IsStandardViewerDisplayed())
            {
                e.CanExecute = IsImageDisplayed();
            }

            if (IsThreeSixtyViewerDisplayed())
            {
                e.CanExecute = IsImageDisplayed() && ThreeSixtyViewer.CanAutoRotateLeft();
            }
        }

        private void CmdRotateLeftExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsStandardViewerDisplayed())
            {
                StandardViewer.RotateLeft();
            }

            if (IsThreeSixtyViewerDisplayed())
            {
                ThreeSixtyViewer.AutoRotateLeft();
                bubbleViewer.ShowTemporaryBubble("Rotation speed: " + Math.Abs(ThreeSixtyViewer.GetAutoRotateSpeed()));
            }
        }

        #endregion

        #region CmdRotateRight

        public static RoutedCommand CmdRotateRight = new RoutedCommand();

        private void CmdRotateRightCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            if (IsStandardViewerDisplayed())
            {
                e.CanExecute = IsImageDisplayed();
            }

            if (IsThreeSixtyViewerDisplayed())
            {
                e.CanExecute = IsImageDisplayed() && ThreeSixtyViewer.CanAutoRotateRight();
            }
        }

        private void CmdRotateRightExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsStandardViewerDisplayed())
            {
                StandardViewer.RotateRight();
            }

            if (IsThreeSixtyViewerDisplayed())
            {
                ThreeSixtyViewer.AutoRotateRight();
                bubbleViewer.ShowTemporaryBubble("Rotation speed: " + Math.Abs(ThreeSixtyViewer.GetAutoRotateSpeed()));
            }
        }

        #endregion

        #region CmdViewReset

        public static RoutedCommand CmdViewReset = new RoutedCommand();

        private void CmdViewResetCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdViewResetExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            if (IsThreeSixtyViewerDisplayed())
            {
                ThreeSixtyViewer.LoadImage(_fileManager.CurrentFile);
            }
            else
            {
                StandardViewer.LoadImage(_fileManager.CurrentFile, false);
            }
        }

        #endregion

        #region CmdViewGrid

        public static RoutedCommand CmdViewGrid = new RoutedCommand();

        private void CmdViewGridCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed() && IsStandardViewerDisplayed();
        }

        private void CmdViewGridExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            StandardViewer.ToggleGrid();
        }

        #endregion

        #region CmdViewFullScreen

        public static RoutedCommand CmdViewFullScreen = new RoutedCommand();

        private void CmdViewFullScreenCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed();
        }

        private void CmdViewFullScreenExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            _fullScreenManager.ToggleFullScreenMode();
        }

        #endregion

        #region CmdExitFullScreen

        public static RoutedCommand CmdExitFullScreen = new RoutedCommand();

        private void CmdExitFullScreenCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = IsImageDisplayed() && _fullScreenManager.IsInFullScreenMode;
        }

        private void CmdExitFullScreenExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            _fullScreenManager.ExitFullScreenMode();
        }

        #endregion

        #region CmdHelpVisitWebsite

        public static RoutedCommand CmdHelpVisitWebsite = new RoutedCommand();

        private void CmdHelpVisitWebsiteCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdHelpVisitWebsiteExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            Process.Start(new ProcessStartInfo(@"https://www.kuk360.com/"));
        }

        #endregion

        #region CmdHelpCheckForUpdates

        public static RoutedCommand CmdHelpCheckForUpdates = new RoutedCommand();

        private void CmdHelpCheckForUpdatesCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdHelpCheckForUpdatesExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            try
            {
                WaitWindow.Execute(this, AppInfo.DownloadLatestVersionInfo, 2);
                LatestVersionInfo latestVersionInfo = AppInfo.LatestVersionInfo;

                Version thisVersion = new Version(AppInfo.Version);
                Version latestVersion = new Version(latestVersionInfo.Version);

                if (0 > thisVersion.CompareTo(latestVersion))
                {
                    if (MessageBoxUtils.AskQuestion(this, "Application update is available." + Environment.NewLine + "Do you want to open the downloads page?") == MessageBoxResult.Yes)
                    {
                        Uri uri = new Uri(latestVersionInfo.DownloadUrl, UriKind.Absolute);
                        if ((!uri.IsAbsoluteUri) || ((uri.Scheme != Uri.UriSchemeHttp) && (uri.Scheme != Uri.UriSchemeHttps)))
                            throw new Exception("Invalid update URL");

                        Process.Start(new ProcessStartInfo(latestVersionInfo.DownloadUrl));
                    }
                }
                else
                {
                    MessageBoxUtils.ShowInfo(this, "Application is up to date.");
                }
            }
            catch (BackgroundWorkerException ex)
            {
                MessageBoxUtils.ShowError(this, ex.InnerException);
            }
            catch (Exception ex)
            {
                MessageBoxUtils.ShowError(this, ex);
            }
        }

        #endregion

        #region CmdHelpAbout

        public static RoutedCommand CmdHelpAbout = new RoutedCommand();

        private void CmdHelpAboutCanExecute(object sender, CanExecuteRoutedEventArgs e)
        {
            e.CanExecute = true;
        }

        private void CmdHelpAboutExecuted(object sender, ExecutedRoutedEventArgs e)
        {
            AboutWindow aboutWindow = new AboutWindow();
            aboutWindow.Owner = this;
            aboutWindow.ShowDialog();
        }

        #endregion
    }
}
