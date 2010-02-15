/****************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt3D module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** No Commercial Usage
** This file contains pre-release code and may not be distributed.
** You may use this file in accordance with the terms and conditions
** contained in the Technology Preview License Agreement accompanying
** this package.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights.  These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**
**
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtTest/QtTest>
#include <QtGui/qimage.h>
#include "qclcontext.h"

const int Test_Width = 1024;
const int Test_Height = 1024;
const int Test_MaxIterations = 512;

class tst_Mandelbrot : public QObject
{
    Q_OBJECT
public:
    tst_Mandelbrot() {}
    virtual ~tst_Mandelbrot() {}

private slots:
    void initTestCase();
    void plain_data();
    void plain();
    void buildProgram();
    void openclPerPixel_data();
    void openclPerPixel();
    void openclPerPixelHost_data();
    void openclPerPixelHost();
    void openclPerGroup_data();
    void openclPerGroup();

private:
    QCLContext context;
    QCLProgram program;
    QCLKernel perPixelKernel;
    QCLKernel perGroupKernel;
};

void tst_Mandelbrot::initTestCase()
{
    QVERIFY(context.create());
}

// Generate Mandelbrot iteration data using plain C++ and no acceleration.
void tst_Mandelbrot::plain_data()
{
    QTest::addColumn<qreal>("centerx");
    QTest::addColumn<qreal>("centery");
    QTest::addColumn<qreal>("diameter");

    // Defines some of the steps in the 14-step Mandelbrot zoom at
    // http://commons.wikimedia.org/wiki/Mandelbrot_set
    QTest::newRow("full")
        << qreal(-0.7f) << qreal(0.0f) << qreal(3.0769f);
    QTest::newRow("zoom1")
        << qreal(-0.87591f) << qreal(0.20464f) << qreal(0.53184f);
    QTest::newRow("zoom2")
        << qreal(-0.759856f) << qreal(0.125547f) << qreal(0.051579f);
}
void tst_Mandelbrot::plain()
{
    QFETCH(qreal, centerx);
    QFETCH(qreal, centery);
    QFETCH(qreal, diameter);

    QRectF region(centerx - diameter / 2, centery - diameter / 2,
                  diameter, diameter);

    QImage data(Test_Width, Test_Height, QImage::Format_RGB32);
    QBENCHMARK {
        int width = data.width();
        int height = data.height();
        float xstep = region.width() / width;
        float ystep = region.height() / height;
        float yin = region.y();
        uint *line = reinterpret_cast<uint *>(data.bits());
        int stride = data.bytesPerLine() / sizeof(uint);
        for (int ypos = 0; ypos < height; ++ypos, yin += ystep) {
            float xin = region.x();
            for (int xpos = 0; xpos < width; ++xpos, xin += xstep) {
                int iteration = 0;
                float x = 0;
                float y = 0;
                while (iteration < Test_MaxIterations) {
                    float x2 = x * x;
                    float y2 = y * y;
                    if ((x2 + y2) > (2.0f * 2.0f))
                        break;
                    float xtemp = x2 - y2 + xin;
                    y = 2 * x * y + yin;
                    x = xtemp;
                    ++iteration;
                }
                line[xpos] = iteration;
            }
            line += stride;
        }
    }
}

void tst_Mandelbrot::buildProgram()
{
    QBENCHMARK_ONCE {
        program = context.buildProgramFromSourceFile
            (QLatin1String(":/mandelbrot.cl"));
        perPixelKernel = program.createKernel("mandelbrot_per_pixel");
        perGroupKernel = program.createKernel("mandelbrot_per_group");
    }
}

// Generate Mandelbrot iteration data using a per-pixel OpenCL kernel.
void tst_Mandelbrot::openclPerPixel_data()
{
    plain_data();
}
void tst_Mandelbrot::openclPerPixel()
{
    QFETCH(qreal, centerx);
    QFETCH(qreal, centery);
    QFETCH(qreal, diameter);

    QVERIFY(!perPixelKernel.isNull());

    QRectF region(centerx - diameter / 2, centery - diameter / 2,
                  diameter, diameter);

    QCLBuffer data = context.createBufferDevice
        (Test_Width * Test_Height * sizeof(int), QCLBuffer::WriteOnly);
    perPixelKernel.setGlobalWorkSize(QCLWorkSize(Test_Width, Test_Height));

    QBENCHMARK {
        QCLEvent event = perPixelKernel
            (data, float(region.x()), float(region.y()),
             float(region.width()), float(region.height()),
             Test_Width, Test_Height, Test_MaxIterations);
        event.wait();
    }
}

// Generate Mandelbrot iteration data using a per-pixel OpenCL kernel,
// but write into host-accessible memory.
void tst_Mandelbrot::openclPerPixelHost_data()
{
    plain_data();
}
void tst_Mandelbrot::openclPerPixelHost()
{
    QFETCH(qreal, centerx);
    QFETCH(qreal, centery);
    QFETCH(qreal, diameter);

    QVERIFY(!perPixelKernel.isNull());

    QRectF region(centerx - diameter / 2, centery - diameter / 2,
                  diameter, diameter);

    QCLBuffer data = context.createBufferHost
        (0, Test_Width * Test_Height * sizeof(int), QCLBuffer::WriteOnly);
    perPixelKernel.setGlobalWorkSize(QCLWorkSize(Test_Width, Test_Height));

    QBENCHMARK {
        QCLEvent event = perPixelKernel
            (data, float(region.x()), float(region.y()),
             float(region.width()), float(region.height()),
             Test_Width, Test_Height, Test_MaxIterations);
        event.wait();
    }
}

// Generate Mandelbrot iteration data using a 16x16 per-group OpenCL kernel.
// This tests whether doing more pixels per kernel call helps or hinders.
void tst_Mandelbrot::openclPerGroup_data()
{
    plain_data();
}
void tst_Mandelbrot::openclPerGroup()
{
    QFETCH(qreal, centerx);
    QFETCH(qreal, centery);
    QFETCH(qreal, diameter);

    QVERIFY(!perGroupKernel.isNull());

    QRectF region(centerx - diameter / 2, centery - diameter / 2,
                  diameter, diameter);

    QCLBuffer data = context.createBufferDevice
        (Test_Width * Test_Height * sizeof(int), QCLBuffer::WriteOnly);
    perGroupKernel.setGlobalWorkSize
        (QCLWorkSize(Test_Width / 16, Test_Height / 16));

    QBENCHMARK {
        QCLEvent event = perGroupKernel
            (data, float(region.x()), float(region.y()),
             float(region.width()), float(region.height()),
             Test_Width, Test_Height, Test_MaxIterations, 16);
        event.wait();
    }
}

QTEST_MAIN(tst_Mandelbrot)

#include "tst_mandelbrot.moc"