//Источни: https://narfu.ru/upload/iblock/5fb/2_2_4a.pdf
#include <cmath>
#include <QtCharts/QtCharts>
#include <QtCharts/QBarSet>
#include <QtCharts/QBarSeries>
#include <iostream>
/*
 * (1)
 * Сила Лоренца: Fл = qvB
 * По 2 закону Ньютона ma = qvB =>
 * mv^2/r = qvB => mv/r = qB
 * (2)
 * Так как скорость частицы перпендикулярна магнитному полю, то
 * энергия скорости равна работе эл. поля: mv^2/2 = qU
 *=> v^2 = 2qU/m
 * (3) Из 1 и 2 получим B = m * sqrt(2qU/m) / r
 * Так же знаем что  B = Ic * n * m0(магнитная постоянная)
 * подставляем и получаем:
 * r = m * sqrt(2eU/m) * 1/(e * Ic*n*m0)
 */

using namespace std;
namespace MagnetronModel {
    inline double e = 1.602176634 * std::pow(10, -19); //  Заряд электрона
    inline double m_e = 9.109 * std::pow(10, -31);  // Масса электрона
    inline double m0 = 4 * M_PI * std::pow(10, -7); //Магнитная постоянная

    inline double count_Ic(double U, int n, double R_a, double R_k) {
        return sqrt((8 * m_e * U) / (e * pow((R_a - R_k), 2))) * 1 / (m0 * n);
    }

    inline double getRadius(double U, double Ic, int n) {
        return m_e * sqrt(2 * e * U / m_e) * 1 / (e * Ic * n * m0);
    }

    inline int Magnetron(int argc, char *argv[]) {
        QApplication a(argc, argv);

        double R_k; // Радиус катода
        double R_a; // Радиус анода
        double U; // Напряжение для нахождения Iкр
        double U1, U2; // Диапазон напряжений
        int n; // Количество витков на единицу длины
        double Ic; // сила тока для поиска радиуса траектории при напряжении U
        bool touched = false;

        std::cout << "Input: Rk, Ra, U, n, Ic(for path)." << '\n';
        std::cin >> R_k >> R_a >> U >> n >> Ic;
        std::cout << "Input U1 and U2 for diagramm." << '\n';
        std::cin >> U1 >> U2;

        double Ikr = count_Ic(U, n, R_a, R_k); // Находим критический ток
        std::cout << "Ikr (Ra,Rk): " << Ikr << '\n';
        double R_p = getRadius(U, Ic, n); // Нашли радиус при заданном U и Ic
        std::cout << "R_p: " << R_p << '\n';
        double R = U /
                   Ikr; // Сопротивление соленоида для условия, что при заданном Uкр мы находим Iкр при которой радиус равен R_a - R_k
        std::cout << "R(Om)" << R << '\n';

        auto *chart = new QChart();
        auto *chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        auto *series_k1 = new QLineSeries();
        auto *series_k2 = new QLineSeries();

        auto *series_a1 = new QLineSeries();
        auto *series_a2 = new QLineSeries();

        auto *series_p = new QLineSeries();
        for (int i = 270; i <= 630; ++i) {
            double x_k1 = R_k * 0.01 * std::cos(M_PI / 180 * i);
            double y_k1 = R_k * 0.01 * std::sin(M_PI / 180 * i);
            series_k1->append(x_k1, y_k1);
            double x_k2 = R_k * std::cos(M_PI / 180 * i);
            double y_k2 = R_k * std::sin(M_PI / 180 * i);
            series_k2->append(x_k2, y_k2);

            double x_a1 = R_a * std::cos(M_PI / 180 * i);
            double y_a1 = R_a * std::sin(M_PI / 180 * i);
            series_a1->append(x_a1, y_a1);
            double x_a2 = (R_a + R_p) * std::cos(M_PI / 180 * i);
            double y_a2 = (R_a + R_p) * std::sin(M_PI / 180 * i);
            series_a2->append(x_a2, y_a2);

            double x_p = R_p * std::cos(M_PI / 180 * i);
            double y_p = R_p * std::sin(M_PI / 180 * i);
            if (!touched) {
                series_p->append(x_p, y_p + R_k / 2 + R_p);
            }
            if (!(y_p + R_k / 2 + R_p <= R_a || 2 * R_p < R_a - R_k)) {
                touched = true;
            }
        }

        chart->addSeries(series_p);
        chart->addSeries(new QAreaSeries(series_k2, series_k1));
        chart->addSeries(new QAreaSeries(series_a2, series_a1));

        chart->createDefaultAxes();
        chart->axisX()->setTitleText("X");
        chart->axisY()->setTitleText("Y");

        chart->setTitle("electron path");
        auto barSetU = new QBarSet("U");
        auto barSetI = new QBarSet("I");
        auto barSeries = new QBarSeries();
        while (U1 < U2) {
            Ic = count_Ic(U1, n, R_a, R_k);
            auto ic = U1 / R; // по закону Ома ищем силу тока при данном Напряжение
            if (ic == Ic) {
                std::cout << "Electron makes round" << U1 << '\n';
            }
            barSetI->append(ic);
            ++U1;
        }
        barSeries->append(barSetU);
        barSeries->append(barSetI);
        auto *chart2 = new QChart();
        chart2->addSeries(barSeries);
        chart2->setTitle("Ic(U)");
        chart2->setAnimationOptions(QChart::SeriesAnimations);

        auto *axisY = new QValueAxis();
        auto *axisX = new QBarCategoryAxis();
        chart2->addAxis(axisX, Qt::AlignBottom);
        chart2->addAxis(axisY, Qt::AlignLeft);
        barSeries->attachAxis(axisX);
        barSeries->attachAxis(axisY);


        auto *chartView2 = new QChartView(chart2);
        chartView->setRenderHint(QPainter::Antialiasing);
        auto *layout = new QHBoxLayout();
        layout->addWidget(chartView);
        layout->addWidget(chartView2);
        auto widget = new QWidget();
        widget->setLayout(layout);

        QMainWindow window;
        window.setCentralWidget(widget);
        window.resize(1280, 720);
        window.show();

        return a.exec();
    }
}