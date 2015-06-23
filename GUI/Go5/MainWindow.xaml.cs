using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.Threading;
using System.Runtime.InteropServices;
using System.IO;
using System.Globalization;

namespace Go5
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        const string dllPath = @"E:\Codes\CrazyGo\Release\CrazyGo.dll";
                                //@"E:\Codes\CrazyGo_Trail\Release\CrazyGo.dll";
                                //@"E:\Codes\AI\Release\AI.dll";
                                //@"E:\Codes\AI\Debug\AI.dll";
        private int[,] chessboard = new int[15, 15];
        private int turn = 1;
        private bool okToMove = false;
        private int sum = 0;
        private Stack<Point> moves = new Stack<Point>();
        private Stack<int> controls = new Stack<int>();
        private delegate void delegateForUI(int x, int y);
        private delegate int JustOneParam(int mv);
        private delegate void WithoutParam();
        private delegate int DefensiveDelegate();
        public MainWindow()
        {
            InitializeComponent();
        }
        private void Offensive_Click(object sender, RoutedEventArgs e)
        {
            Dispatcher.Invoke(new WithoutParam(GetOffensive));
            OffensiveButton.IsEnabled = false;
            DefensiveButton.IsEnabled = false;
        }
        private void Defensive_Click(object sender, RoutedEventArgs e)
        {
            int mv = (int)Dispatcher.Invoke(new DefensiveDelegate(GetDefensive));
            Update(mv / 15, mv % 15);
            OffensiveButton.IsEnabled = false;
            DefensiveButton.IsEnabled = false;
        }
        private void NewGame_Click(object sender, RoutedEventArgs e)
        {
            Dispatcher.Invoke(new WithoutParam(NewGame));
            okToMove = true;
            OffensiveButton.IsEnabled = true;
            DefensiveButton.IsEnabled = true;
            Dynamix.Children.Clear();
            chessboard = new int[15, 15];
            moves.Clear();
            controls.Clear();
            sum = 0;
            turn = 1;
        }
        private void Retract_Click(object sender, RoutedEventArgs e)
        {
            System.Media.SoundPlayer player = new System.Media.SoundPlayer(Go5.Properties.Resources.fun);
            player.Play();
            //int success = (int)Dispatcher.Invoke(new DefensiveDelegate(CancleMove));
            //if (success == 1 && moves.Count >= 4)
            //{
            //    Point point;
            //    point = moves.Pop();
            //    chessboard[(int)point.X, (int)point.Y] = 0;
            //    Dynamix.Children.RemoveAt(controls.Pop());
            //    Dynamix.Children.RemoveAt(controls.Pop());
            //    point = moves.Pop();
            //    chessboard[(int)point.X, (int)point.Y] = 0;
            //    Dynamix.Children.RemoveAt(controls.Pop());
            //    Dynamix.Children.RemoveAt(controls.Pop());
            //}
        }
        private void Image_MouseLeftButtonUp(object sender, MouseButtonEventArgs e)
        {
            if (!okToMove)
            {
                return;
            }
            Point mouse_point = e.GetPosition(e.Source as FrameworkElement);
            int image_width = 385;
            int edge_length = 25;
            int XofMouse = (int)mouse_point.X;
            int YofMouse = (int)mouse_point.Y;
            // 判断是否落在棋盘内
            if (XofMouse <= 5 || XofMouse >= image_width - 5 ||
                YofMouse <= 5 || YofMouse >= image_width - 5)
            {
                return;
            }
            // 判断是否落在棋盘的有效区域
            int x_from_cross = (XofMouse - 5) % edge_length;
            int y_from_cross = (YofMouse - 5) % edge_length;
            if (x_from_cross > edge_length * 3 / 2 && x_from_cross < edge_length
                && y_from_cross > edge_length * 3 / 2 && y_from_cross < edge_length)
            {
                return;
            }
            // 求坐标
            int x_coordinate = (XofMouse - 5) / edge_length;
            int y_coordinate = (YofMouse - 5) / edge_length;
            // 判断是否已有落子
            if (chessboard[x_coordinate, y_coordinate] > 0)
            {
                return;
            }
            // 画图
            Update(x_coordinate, y_coordinate);
            Thread worker = new Thread(CalcMove);
            worker.Start(x_coordinate * 15 + y_coordinate);
        }
        private void Image_MouseMove(object sender, MouseEventArgs e)
        {
            Point mouse_point = e.GetPosition(e.Source as FrameworkElement);
            this.StatusText.Text = mouse_point.ToString();
        }
        private void Image_MouseLeave(object sender, MouseEventArgs e)
        {
            this.StatusText.Text = "离开";
        }
        private void CalcMove(Object obj)
        {
            okToMove = false;
            int mv = (int)Dispatcher.Invoke(new JustOneParam(MakeMove), (int)obj);
            // 玩家落子后本局结束
            if (mv == 1024 || mv == 2048 || mv == 4096)
            {
                Dispatcher.BeginInvoke(new JustOneParam(GameOver), mv);
                return;
            }
            // AI落子后本局结束
            else if (mv >= 1024)
            {
                int flag = mv < 2048 ? 1024 : mv < 4096 ? 2048 : 4096;
                mv = mv < 2048 ? mv - 1024 : mv < 4096 ? mv - 2048 : mv - 4096;
                chessboard[mv / 15, mv % 15] = turn;
                Dispatcher.BeginInvoke(new delegateForUI(Update), mv / 15, mv % 15);
                Dispatcher.BeginInvoke(new JustOneParam(GameOver), flag);
                return;
            }
            else
            {
                chessboard[mv / 15, mv % 15] = turn;
                Dispatcher.BeginInvoke(new delegateForUI(Update), mv / 15, mv % 15);
                okToMove = true;
            }
            Dispatcher.BeginInvoke(new WithoutParam(PrintInfo));
        }
        private void Update(int x, int y)
        {
            chessboard[x, y] = turn;
            moves.Push(new Point(x, y));
            // 画图
            Ellipse piece = new Ellipse();
            piece.Width = 22;
            piece.Height = 22;
            piece.Visibility = Visibility.Visible;
            piece.Margin = new Thickness(17 + x * 25 - 11, 
                17 + y * 25 - 11, 0, 0);
            piece.Fill = (RadialGradientBrush)Application.Current.
                FindResource(turn == 1 ? "BlackRadian" : "WhiteRadian");
            controls.Push(Dynamix.Children.Add(piece));
            sum += 1;
            // 取得文本宽度及高度
            FormattedText tmp = new FormattedText(sum.ToString(),
                CultureInfo.CurrentUICulture,
                FlowDirection.LeftToRight,
                new Typeface(this.FontFamily, this.FontStyle, this.FontWeight, this.FontStretch),
                this.FontSize,
                Brushes.Black);
            // 标上序号
            TextBlock number = new TextBlock();
            number.Background = Brushes.Transparent;
            number.Foreground = turn == 1 ? Brushes.White : Brushes.Black;
            number.Text = sum.ToString();
            number.Margin = new Thickness(17 + x * 25 - tmp.Width / 2,
                17 + y * 25 - tmp.Height / 2, 0, 0);
            controls.Push(Dynamix.Children.Add(number));
            turn ^= 3;
        }
        private void PrintInfo()
        {
            byte[] buffer;
            ReadInfo(out buffer);
            this.StatisticText.Text += System.Text.Encoding.Default.GetString(buffer, 0, buffer.Length);
            this.StatisticText.ScrollToEnd();
        }
        private int GameOver(int x)
        {
            switch(x)
            {
                case 1024:
                    MessageBox.Show("黑方胜");
                    break;
                case 2048:
                    MessageBox.Show("白方胜");
                    break;
                case 4096:
                    MessageBox.Show("平局");
                    break;
                default:
                    MessageBox.Show("Fuxking Problem Happened !");
                    break;
            }
            return 0;
        }
        // 落子及机器后台计算
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern int MakeMove(int mv);
        // 悔棋
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern int CancleMove();
        // 先手
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern void GetOffensive();
        // 后手
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern int GetDefensive();
        // 新局
        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        private static extern void NewGame();

        [StructLayout(LayoutKind.Sequential)]
        struct RunInfo
        {
            public IntPtr data;
            public int len;
        };

        [DllImport(dllPath, CallingConvention = CallingConvention.Cdecl)]
        unsafe private static extern int FetchInfo(RunInfo* outData);

        unsafe static int ReadInfo(out byte[] buffer)
        {
            RunInfo outData;
            int result = FetchInfo(&outData);
            buffer = new byte[outData.len];
            Marshal.Copy((IntPtr)outData.data, buffer, 0, outData.len);
            return result;
        }
    }
}
