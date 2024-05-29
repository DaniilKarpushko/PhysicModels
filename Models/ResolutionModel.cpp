#include <QApplication>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>
#include <QtMath>
#include <vector>
#include <iostream>

namespace ResolutionModel {
    inline double norm(double I, double Imax) {
        return I / Imax;
    }

    inline double intensity(double d, double lambda, int N, double fi) {
        return pow(sin(N * (M_PI * d * sin(fi) / lambda)) / (N * sin(M_PI * d * sin(fi) / lambda)), 2);
    }

    inline QColor mixColors(const std::vector<QColor> &colors, const std::vector<double> &intensities, double imax) {
        double r = 0, g = 0, b = 0;
        for (size_t i = 0; i < colors.size(); ++i) {
            double normIntensity = norm(intensities[i], imax);
            r += colors[i].red() * normIntensity;
            g += colors[i].green() * normIntensity;
            b += colors[i].blue() * normIntensity;
        }
        return QColor(static_cast<int>(qMin(r, 255.0)), static_cast<int>(qMin(g, 255.0)),
                      static_cast<int>(qMin(b, 255.0)));
    }

    inline QColor wavelengthToColor(double wavelength) {
        double R, G, B;

        if (wavelength >= 380 && wavelength < 440) {
            R = -(wavelength - 440) / (440 - 380);
            G = 0.0;
            B = 1.0;
        } else if (wavelength >= 440 && wavelength < 490) {
            R = 0.0;
            G = (wavelength - 440) / (490 - 440);
            B = 1.0;
        } else if (wavelength >= 490 && wavelength < 510) {
            R = 0.0;
            G = 1.0;
            B = -(wavelength - 510) / (510 - 490);
        } else if (wavelength >= 510 && wavelength < 580) {
            R = (wavelength - 510) / (580 - 510);
            G = 1.0;
            B = 0.0;
        } else if (wavelength >= 580 && wavelength < 645) {
            R = 1.0;
            G = -(wavelength - 645) / (645 - 580);
            B = 0.0;
        } else if (wavelength >= 645 && wavelength <= 750) {
            R = 1.0;
            G = 0.0;
            B = 0.0;
        } else {
            R = 0.0;
            G = 0.0;
            B = 0.0;
        }

        double factor = 1.0;
        if (wavelength >= 380 && wavelength < 420) {
            factor = 0.3 + 0.7 * (wavelength - 380) / (420 - 380);
        } else if (wavelength >= 645 && wavelength <= 750) {
            factor = 0.3 + 0.7 * (750 - wavelength) / (750 - 645);
        }

        int red = static_cast<int>(255 * R * factor);
        int green = static_cast<int>(255 * G * factor);
        int blue = static_cast<int>(255 * B * factor);

        return QColor(red, green, blue);
    }

    inline int run(int argc, char **argv) {
        QApplication a(argc, argv);

        int d; // период решетки в нанометрах
        int N; // количество штрихов

        std::cout << "Input period and number" << std::endl;
        std::cin >> d >> N;

        std::vector<double> wavelengths;
        for (double lambda = 380; lambda <= 750; lambda += 1) {
            wavelengths.push_back(lambda);
        }

        std::vector<std::vector<double>> intensities(wavelengths.size());
        std::vector<double> angles;

        for (double angle = qDegreesToRadians(-89.9); angle < qDegreesToRadians(90); angle += qDegreesToRadians(0.1)) {
            angles.push_back(angle);
            for (size_t i = 0; i < wavelengths.size(); ++i) {
                intensities[i].push_back(intensity(d, wavelengths[i], N, angle));
            }
        }

        int width = 1800;
        int height = 324;

        QImage image(width, height, QImage::Format_RGB32);

        for (int x = 0; x < width; ++x) {
            std::vector<QColor> colors;
            std::vector<double> color_intensities;
            for (size_t i = 0; i < wavelengths.size(); ++i) {
                colors.push_back(wavelengthToColor(wavelengths[i]));
                color_intensities.push_back(intensities[i][x]);
            }
            QColor color = mixColors(colors, color_intensities, 8);
            for (int y = 0; y < height; ++y) {
                image.setPixel(x, y, color.rgb());
            }
        }

        auto label = new QLabel();
        label->setPixmap(QPixmap::fromImage(image));
        label->setAlignment(Qt::AlignCenter);
        label->setBaseSize(900, 324);

        auto chart = new QChart();
        for (size_t i = 0;
             i < wavelengths.size(); i += 50) {
            auto series = new QLineSeries();
            series->setColor(wavelengthToColor(wavelengths[i]));
            for (size_t j = 0; j < angles.size(); ++j) {
                series->append(qRadiansToDegrees(angles[j]), intensities[i][j]);
            }
            chart->addSeries(series);
        }
        auto chart_view = new QChartView(chart);
        chart_view->setRenderHint(QPainter::Antialiasing);
        chart->createDefaultAxes();
        chart->axisX()->setTitleText("Angle");
        chart->axisY()->setTitleText("Intensity");

        auto layout = new QVBoxLayout();
        layout->addWidget(chart_view);
        layout->addWidget(label);

        auto widget = new QWidget();
        widget->setLayout(layout);

        QMainWindow window;
        window.setCentralWidget(widget);
        window.resize(900, 700);
        window.show();

        return a.exec();
    }
}