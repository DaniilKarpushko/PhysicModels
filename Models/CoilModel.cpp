/* Вывод формул:
 * n = L / d где L - длина катушки, а d - это диаметра провода
 * или же n = l / (pi*D) - где D - диаметр катушки
 * а l - длина провода
 * Формула индуктивности:
 * L = Fi(B) / I = N * двойной интеграл BdS / I
 * Fi(B) - поток через катушку
 * B = m0 * m * N / l * I =>
 * получим Fi(B) = N * B * S = m0 * m * N^2 / l * I * S
 * L = Fi(B) / I = m0 * m * N^2 / l * S
 *
 * test data: 10 0.001 0.05 0.2
 */
// L = 0.008 * D^2 * N ^ 2 / (3D + gh + 10g)
#include <QApplication>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include<QMainWindow>

#include <cmath>
#include <iostream>

namespace CoilModel {
    inline double m0 = 4 * M_PI * pow(10, -7);

    inline int Coil(int argc, char *argv[]) {
        QApplication app(argc, argv);

        double l; // длина провода
        double d; // диаметр провода
        double L; // длина катушки
        double D; // диаметр катушки

        std::cout << "Enter: wire length, wire diameter, coil length, coil diameter - all in meters" << '\n';
        std::cin >> l >> d >> L >> D;

        int n = (int) std::min(l / (M_PI * D), L / d);
        int n2 = (int) (l / (L / d * M_PI * D)); // толщина
        if (n2 > 1) {
            double L_ind2 = 31.6 * pow(D, 2) * pow(n, 2) / (3 * D + 9 * n * n2 * M_PI * D + 10 * n2 * d);
            std::cout << "Induction with several layers " << L_ind2 << " layers: " << n2 << '\n';
        }

        double L_ind = m0 * pow(n, 2) / L * M_PI * pow((D / 2), 2);

        std::cout << "Number of turns: " << n << " Inductance: " << L_ind << '\n';

        auto *series_B = new QLineSeries();
        series_B->setName("B(l)");

        for (double i = 0.001; i <= L; i += 0.001) {
            double B = m0 * n / i;
            series_B->append(B, i);
        }

        auto *chart = new QChart();
        auto chart_view = new QChartView(chart);
        chart_view->setRenderHint(QPainter::Antialiasing);

        chart->addSeries(series_B);

        chart->createDefaultAxes();
        chart->axisX()->setTitleText("Coil length(meters)");
        chart->axisY()->setTitleText("B/I");

        QMainWindow window;
        window.setCentralWidget(chart_view);
        window.resize(720, 360);
        window.show();

        return app.exec();
    }
}
