#include <iostream>
#include <vector>

#include <QApplication>
#include <QtCharts/QLineSeries>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QMainWindow>
#include <QHBoxLayout>
#include <QLabel>
#include <random>

using namespace std;
namespace InterferenceMode {
    inline double norm(double I, double Imax) {
        double kf = 1;
        while (kf < Imax / kf) {
            kf *= 10;
        }
        return (I - (Imax - kf)) / kf;
    }

    inline double intensity(double fi, int N, double a, double b, double lambda) {
        return (pow(sin(M_PI * b * sin(fi) / lambda), 2) * pow(sin(N * M_PI * (a + b) * sin(fi) / lambda), 2))
               / (pow((M_PI * b * sin(fi) / lambda), 2) * pow(sin(M_PI * (a + b) * sin(fi) / lambda), 2));
    }

//4 100 400 1 300
    inline int run(int argc, char *argv[]) {
        QApplication app(argc, argv);

        char type;
        int N; // количество щелей 1-10
        double b; // ширина щелей
        double d; // период
        double l; // расстояние до решетки
        double max_intensity = -1;

        std::cout << "Input type: m - mono, k - quasi-mono" << '\n';
        std::cin >> type;

        std::cout << "Input: n width(nm) period(nm) distance(m)" << '\n';
        std::cin >> N >> b >> d >> l;

        double a = d - b;

        auto *seriesA = new QLineSeries();
        auto *seriesB = new QLineSeries();

        vector<double> points;
        vector<double> angles;
        vector<double> angle_intensities;
        vector<double> point_intensities;
        vector<double> lambdas;

        if (type == 'm') {
            double lambda; // длина волны

            std::cout << "lambda(nm): " << '\n';
            std::cin >> lambda;

            for (double i = qDegreesToRadians(-90); i < qDegreesToRadians(90); i += qDegreesToRadians(0.1)) {
                auto _intensity = intensity(i, N, a, b, lambda);
                angle_intensities.emplace_back(_intensity);
                angles.emplace_back(i);
                if (abs(i) < qDegreesToRadians(60)) {
                    points.emplace_back(tan(i) * l);
                    point_intensities.emplace_back(_intensity);
                }
                if (_intensity > max_intensity) {
                    max_intensity = _intensity;
                }
            }
        } else if (type == 'k') {
            double lambda_center, lambda_width;
            std::cout << "lambda_center(nm) lambda_width(nm): " << '\n';
            std::cin >> lambda_center >> lambda_width;

            std::mt19937 gen(std::random_device{}());
            std::uniform_real_distribution<> dis((lambda_center - lambda_width / 2),
                                                 (lambda_center + lambda_width / 2));
            int num_samples = 100;
            for (int i = 0; i < num_samples; ++i) {
                lambdas.push_back(dis(gen));
            }

            for (auto lambda: lambdas) {
                int ptr1 = 0;
                int ptr2 = 0;
                for (double i = qDegreesToRadians(-90); i < qDegreesToRadians(90); i += qDegreesToRadians(0.1)) {
                    auto _intensity = intensity(i, N, a, b, lambda);
                    if (angles.size() < 1800) {
                        angles.emplace_back(i);
                        angle_intensities.emplace_back(_intensity);
                    } else {
                        angle_intensities[ptr1] += _intensity;
                        ptr1++;
                    }
                    if (abs(i) < qDegreesToRadians(60)) {
                        if (points.size() < 1200) {
                            points.emplace_back(tan(i) * l);
                            point_intensities.emplace_back(_intensity);
                        } else {
                            point_intensities[ptr2] += _intensity;
                            ptr2++;
                        }
                    }
                    if (_intensity > max_intensity) {
                        max_intensity = _intensity;
                    }
                }
            }
        }

        for (int i = 0; i < angles.size(); ++i) {
            seriesA->append(angles[i], angle_intensities[i]);
        }

        for (int i = 0; i < points.size(); ++i) {
            seriesB->append(points[i], point_intensities[i]);
        }

        auto chartA = new QChart();
        auto chart_viewA = new QChartView(chartA);
        chart_viewA->setRenderHint(QPainter::Antialiasing);
        chartA->addSeries(seriesA);
        chartA->createDefaultAxes();
        chartA->axisX()->setTitleText("angle");
        chartA->axisY()->setTitleText("intensity");

        auto chartB = new QChart();
        auto chart_viewB = new QChartView(chartB);
        chart_viewB->setRenderHint(QPainter::Antialiasing);
        chartB->addSeries(seriesB);
        chartB->createDefaultAxes();
        chartB->axisX()->setTitleText("points");
        chartB->axisY()->setTitleText("intensity");

        auto layout = new QHBoxLayout();
        auto mainLayout = new QVBoxLayout();
        layout->addWidget(chart_viewA);
        layout->addWidget(chart_viewB);
        auto widget = new QWidget();
        widget->setLayout(layout);

        int width = 600;
        int height = 324;

        QImage image(width, height, QImage::Format_RGB32);

        std::cout << max_intensity << '\n';
        for (int x = 0; x < width; ++x) {
            auto I = point_intensities[x + 300];
            int colorValue = static_cast<int>(255 * norm(I, max_intensity));
            for (int y = 0; y < height; ++y) {
                image.setPixel(x, y, qRgb(colorValue, 0, 255 - colorValue));
            }
        }

        auto label = new QLabel();
        label->setPixmap(QPixmap::fromImage(image));
        label->setAlignment(Qt::AlignCenter);

        mainLayout->addWidget(widget);
        mainLayout->addWidget(label);

        auto mainWidget = new QWidget();
        mainWidget->setLayout(mainLayout);

        QMainWindow window;
        window.setCentralWidget(mainWidget);
        window.resize(720, 360);
        window.show();

        return app.exec();

    }
}