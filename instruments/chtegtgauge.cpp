//////////////////////////////////////////////////////////////////////////
//                                                                      //
// EngineMonitor, a graphical gauge to monitor an aircraft's engine     //
// Copyright (C) 2012 Tobias Rad                                        //
//               2017 Ryan Story                                        //
//                                                                      //
// This program is free software: you can redistribute it and/or modify //
// it under the terms of the GNU General Public License as published by //
// the Free Software Foundation, either version 3 of the License, or    //
// (at your option) any later version.                                  //
//                                                                      //
// This program is distributed in the hope that it will be useful,      //
// but WITHOUT ANY WARRANTY; without even the implied warranty of       //
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        //
// GNU General Public License for more details.                         //
//                                                                      //
// You should have received a copy of the GNU General Public License    //
// along with this program. If not, see <http://www.gnu.org/licenses/>. //
//                                                                      //
//////////////////////////////////////////////////////////////////////////

#include "chtegtgauge.h"

ChtEgt::ChtEgt(QGraphicsObject *parent) : QGraphicsObject(parent)
  , minChtValue(0.0)
  , maxChtValue(0.0)
  , greenYellowChtValue(0.0)
  , yellowRedChtValue(0.0)
  , minEgtValue(0.0)
  , maxEgtValue(0.0)
{
    currentChtValues << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
    currentEgtValues << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
    currentRawChtValues << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;
    currentRawEgtValues << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0 << 0.0;

    chtGauge.setGauge("CHT");

    minChtValue = chtGauge.getMin();
    maxChtValue = chtGauge.getMax();
    minChtLocal = calculateLocalChtValue(minChtValue);
    maxChtLocal = calculateLocalChtValue(maxChtValue);
    numOfRanges = chtGauge.getNRange();

    egtGauge.setGauge("EGT");

    minEgtValue = egtGauge.getMin();
    maxEgtValue = egtGauge.getMax();

    smoothDataCht1.setSampleSize(10);
    smoothDataCht2.setSampleSize(10);

    updateTimer.start(100);

    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(onUpdate()));
}

QRectF ChtEgt::boundingRect() const
{
    return QRectF(-240, -170, 350, 190);
}

/*! \brief Handles drawing of the object
*
* This member handles all of the painting logic used to draw the item and the associated style.
*/
void ChtEgt::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
	//Set Clipping Rect
	painter->setClipRect(boundingRect());

    //Set painter for texts
    //painter->setPen(QPen(Qt::white, 1));
    painter->setFont(QFont("Arial", 16));

    changeFlashState();

    //Draw the static texts
    QRectF chtTitleRect = QRectF(50.0, calculateLocalChtValue(maxChtValue)-25, 60.0, 20.0);
    if (numOfEgt>0) {
        painter->setPen(QPen(QColor(0,255,255), 1));
        painter->drawText(QRectF(-(60*(numOfCht-1))-60, calculateLocalChtValue(minChtValue)+4, 50.0, 20.0), Qt::AlignCenter | Qt::AlignVCenter, "EGT");
    }
    painter->setPen( QPen(Qt::white, 1));
    painter->drawText(QRectF(-(60*(numOfCht-1))-60, calculateLocalChtValue(maxChtValue)-25, 50.0, 23.0), Qt::AlignCenter | Qt::AlignBottom, QString::fromUtf8("°F"));

    //Set painter for texts
    painter->setPen(QPen(Qt::white, 1));
    painter->setFont(QFont("Arial", 18, QFont::Bold));

    //Save thje painter and deactivate Antialising for rectangle drawing
    painter->save();
    painter->setRenderHint(QPainter::Antialiasing, false);
    painter->setRenderHint(QPainter::SmoothPixmapTransform, false);

	//Draw the side legend
	painter->setBrush(Qt::green);
	painter->setPen(QPen(Qt::green, 0));
    painter->drawRect(QRectF(QPointF(60.0, minChtLocal), QPointF(90.0, maxChtLocal)));

    if (chtGauge.getName() != "") {
        j=0;

        for (j=0; j<numOfRanges; j++) {
            startRange = calculateLocalChtValue(chtGauge.definitions[j].start);
            endRange = calculateLocalChtValue(chtGauge.definitions[j].end);
            color = chtGauge.definitions[j].color;

            //Set pen and brush to color and draw the bar
            painter->setPen(color);
            painter->setBrush(color);
            painter->drawRect(QRectF(QPointF(60.0, startRange), QPointF(90.0, endRange)));

            // Draw line extensions for non green ranges
            if (startRange == minChtLocal) {
                painter->drawLine(-(60*(numOfCht-1))-10, endRange, 60, endRange);
            } else {
                painter->drawLine(-(60*(numOfCht-1))-10, startRange-1, 60, startRange-1);
            }

        }
    }

    //Draw the white line to define base of bars
    painter->setPen(Qt::white);
    painter->drawLine(-(60*(numOfCht-1))-30, calculateLocalChtValue(minChtValue)+2, 50, calculateLocalChtValue(minChtValue)+2);

    //Draw the white line to define top of bars
    painter->setPen(Qt::white);
    painter->drawLine(-(60*(numOfCht-1))-10, calculateLocalChtValue(maxChtValue)-2, 90, calculateLocalChtValue(maxChtValue)-2);

    //Draw the white line to show EGT scale
    painter->setPen(Qt::white);
    painter->drawLine(-(60*(numOfCht-1))-30, calculateLocalChtValue(maxChtValue)-2, -(60*(numOfCht-1))-20, calculateLocalChtValue(maxChtValue)-2);
    painter->drawLine(-(60*(numOfCht-1))-30, calculateLocalChtValue(minChtValue)+2, -(60*(numOfCht-1))-30, calculateLocalChtValue(maxChtValue)-2);

    //Draw center lines
    painter->setPen(QPen(Qt::white, 1, Qt::SolidLine));
    for(int i = 0; i < numOfCht; i++)
    {
        painter->drawLine(i*60-((60*(numOfCht-1))-20), -110, i*60-((60*(numOfCht-1))-20), -10);
    }

    painter->restore();

    isAlarmedRed = false;
    isAlarmedYellow = false;

    //Draw the bar graphs
    for(int i=0; i < numOfCht; i++) {
        if (i==0 && !cht1IsValid) {
            painter->setPen(Qt::red);
            painter->drawLine(QPointF(i*60-(60*(numOfCht-1)), -10), QPointF(i*60-((60*(numOfCht-1))-40), maxChtLocal));
            painter->drawLine(QPointF(i*60-(60*(numOfCht-1)-40), -10), QPointF(i*60-((60*(numOfCht-1))), maxChtLocal));
        }

        if (i==1 && !cht2IsValid) {
            painter->setPen(Qt::red);
            painter->drawLine(QPointF(i*60-(60*(numOfCht-1)), -10), QPointF(i*60-((60*(numOfCht-1))-40), maxChtLocal));
            painter->drawLine(QPointF(i*60-(60*(numOfCht-1)-40), -10), QPointF(i*60-((60*(numOfCht-1))), maxChtLocal));
        }

        painter->setBrush(Qt::green);
        painter->setPen(Qt::green);
        cylinderAlarm = 1;

        if ((i==0 && cht1IsValid) || (i==1 && cht2IsValid)) {
            currentLocal = calculateLocalChtValue(currentChtValues.at(i));
            QRectF barRect = QRectF(QPointF(i*60-(60*(numOfCht-1)), -10), QPointF(i*60-((60*(numOfCht-1))-40), currentLocal));

            //Save thje painter and deactivate Antialising for rectangle drawing
            painter->save();
            painter->setRenderHint(QPainter::Antialiasing, false);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, false);

            for (j=0; j<numOfRanges; j++) {

                startRange = calculateLocalChtValue(chtGauge.definitions[j].start);
                endRange = calculateLocalChtValue(chtGauge.definitions[j].end);
                color = chtGauge.definitions[j].color;

                if(currentLocal <= startRange && endRange < currentLocal)
                {
                    //If value is in warning area, bar is drawn red
                    painter->setBrush(color);
                    painter->setPen(color);
                    if (color == Qt::red) {
                        cylinderAlarm = 3;
                        isAlarmedRed = true;
                    } else if (color == Qt::yellow) {
                        cylinderAlarm = 2;

                        if (isAlarmedRed == false) {
                            isAlarmedYellow = true;
                        }
                    }
                }

                if((currentChtValues.at(i) > minChtValue) &&
                        (currentChtValues.at(i) < maxChtValue))
                {
                    //If value is in visible range, draw the bar
                    painter->drawRect(barRect);
                }

            }

            if (currentChtValues.at(i) > maxChtValue)
            {
                barRect = QRectF(QPointF(i*60-(60*(numOfCht-1)), -10), QPointF(i*60-((60*(numOfCht-1))-40), calculateLocalChtValue(maxChtValue)));
                painter->setPen(Qt::red);
                painter->setBrush(Qt::red);
                cylinderAlarm = 3;
                isAlarmedRed = true;
                painter->drawRect(barRect);
            } else if (currentChtValues.at(i) < minChtValue) {
                if (chtGauge.definitions[0].start == minChtValue) {
                    QColor tempColor = chtGauge.definitions[0].color;
                    painter->setPen(tempColor);
                    painter->setBrush(tempColor);
                    if (tempColor == Qt::yellow) {
                        cylinderAlarm = 2;
                        isAlarmedYellow = true;
                    }
                } else {
                    painter->setBrush(Qt::green);
                }
            }

            painter->restore();

            //Define CHT text position and move to current column
            QRectF textRect(-40, -20, 50, 20);
            textRect.moveCenter(QPointF(i*60-((60*(numOfCht-1))-20), -125));
            painter->setFont(QFont("Arial", 18, QFont::Bold));

            if ((isAlarmedRed == true) && (cylinderAlarm == 3)) {
                if (flashState || isAcknowledged) {
                    painter->setPen(Qt::red);
                    painter->setBrush(Qt::red);
                    painter->drawRect(textRect);
                    painter->setPen(Qt::white);
                    painter->drawText(textRect, Qt::AlignCenter, QString::number(currentChtValues.at(i), 'f', 0));

                } else {
                    painter->setPen(Qt::red);
                    painter->drawText(textRect, Qt::AlignCenter, QString::number(currentChtValues.at(i), 'f', 0));
                }

            } else if (cylinderAlarm == 2) {
                if (flashState || isAcknowledged) {
                    painter->setPen(Qt::yellow);
                    painter->setBrush(Qt::yellow);
                    painter->drawRect(textRect);
                    painter->setPen(Qt::black);
                    painter->drawText(textRect, Qt::AlignCenter, QString::number(currentChtValues.at(i), 'f', 0));

                } else {
                    painter->setPen(Qt::yellow);
                    painter->drawText(textRect, Qt::AlignCenter, QString::number(currentChtValues.at(i), 'f', 0));
                }
            } else {
                //Draw the readout
                painter->setPen(Qt::white);
                painter->drawText(textRect, Qt::AlignCenter, QString::number(currentChtValues.at(i), 'f', 0));
            }

            if (numOfEgt > 0 && i < numOfEgt) {
                //Define EGT text position and move to current column
                painter->setPen(QColor(0,255,255));
                QRectF textRectEgt(-45, 65, 65, 20);
                textRectEgt.moveCenter(QPointF(i*60-((60*(numOfCht-1))-20), 5));

                painter->drawText(textRectEgt, Qt::AlignCenter, QString::number(currentEgtValues.at(i), 'f', 0));

                //  Draw the markers for the EGT gauge
                if((currentEgtValues.at(i) > minEgtValue) && (currentEgtValues.at(i) < maxEgtValue))
                {
                    QRectF EgtRect = QRectF(QPointF(i*60-((60*(numOfCht-1))-5), calculateLocalEgtValue(currentEgtValues.value(i))-3), QPointF(i*60-((60*(numOfCht-1))-35), calculateLocalEgtValue(currentEgtValues.value(i))+3));
                    painter->setPen(Qt::black);
                    painter->setBrush(QColor(0,255,255));
                    painter->drawRect(EgtRect);
                    painter->setPen(Qt::white);
                }
            }
        }

        if ((isAlarmedRed == true)) {
            if (flashState || isAcknowledged) {
                painter->setPen(Qt::red);
                painter->setBrush(Qt::red);
                painter->drawRect(chtTitleRect);
                painter->setPen(Qt::white);
                painter->setFont(QFont("Arial", 16));
                painter->drawText(chtTitleRect, Qt::AlignCenter | Qt::AlignVCenter, "CLNT");

            } else {
                painter->setPen(Qt::red);
                painter->setFont(QFont("Arial", 16));
                painter->drawText(chtTitleRect, Qt::AlignCenter | Qt::AlignVCenter, "CLNT");
            }

        } else if ((isAlarmedYellow)) {
            if (flashState || isAcknowledged) {
                painter->setPen(Qt::yellow);
                painter->setBrush(Qt::yellow);
                painter->drawRect(chtTitleRect);
                painter->setFont(QFont("Arial", 16));
                painter->setPen(Qt::black);
                painter->drawText(chtTitleRect, Qt::AlignCenter | Qt::AlignVCenter, "CLNT");

            } else {
                painter->setPen(Qt::yellow);
                painter->setFont(QFont("Arial", 16));
                painter->drawText(chtTitleRect, Qt::AlignCenter | Qt::AlignVCenter, "CLNT");
            }
        } else {
            painter->setPen(Qt::white);
            painter->setFont(QFont("Arial", 16));
            painter->drawText(chtTitleRect, Qt::AlignCenter | Qt::AlignVCenter, "CLNT");
            isAcknowledged = false;
            emit cancelAlarm("CHT");
            isAlarmedRed = false;
            isAlarmedYellow = false;
        }
    }

}

double ChtEgt::calculateLocalChtValue(double value) const
{
    return -(value-minChtValue)/(maxChtValue-minChtValue)*100.0+(-10.0);
}

double ChtEgt::calculateLocalEgtValue(double value) const
{
    return -(value-minEgtValue)/(maxEgtValue-minEgtValue)*100.0+(-10.0);
}

void ChtEgt::addBetweenValue(double value)
{
	betweenValues.append(value);
}

void ChtEgt::setChtValues(double val1, double rawVal1, double val2, double rawVal2, double val3, double rawVal3, double val4, double rawVal4)
{
    if (val1 == -999) {
        cht1IsValid = false;
    } else {
        cht1IsValid = true;
    }

    if (val2 == -999) {
        cht2IsValid = false;
    } else {
        cht2IsValid = true;
    }

    if (val3 == -999) {
        cht3IsValid = false;
    } else {
        cht3IsValid = true;
    }

    if (val4 == -999) {
        cht4IsValid = false;
    } else {
        cht4IsValid = true;
    }

    currentRawChtValues.replace(0, rawVal1);
    currentRawChtValues.replace(1, rawVal2);
    currentRawChtValues.replace(2, rawVal3);
    currentRawChtValues.replace(3, rawVal4);

    if (smooth) {
        currentChtValues.replace(0, smoothDataCht1.dsp_ema_double(val1));
        currentChtValues.replace(1, smoothDataCht2.dsp_ema_double(val2));
        currentChtValues.replace(2, val3);
        currentChtValues.replace(3, val4);
    } else {
        currentChtValues.replace(0, val1);
        currentChtValues.replace(1, val2);
        currentChtValues.replace(2, val3);
        currentChtValues.replace(3, val4);
    }

}

void ChtEgt::setEgtValues(double val1, double rawVal1, double val2, double rawVal2, double val3, double rawVal3, double val4, double rawVal4)
{
    currentRawEgtValues.replace(0, rawVal1);
    currentRawEgtValues.replace(1, rawVal2);
    currentRawEgtValues.replace(2, rawVal3);
    currentRawEgtValues.replace(3, rawVal4);

    currentEgtValues.replace(0, val1);
    currentEgtValues.replace(1, val2);
    currentEgtValues.replace(2, val3);
    currentEgtValues.replace(3, val4);

}

void ChtEgt::setBorders(double minimum, double maximum, double yellowBorder, double redBorder, double minEgt, double maxEgt)
{
    minChtValue = minimum;
    maxChtValue = maximum;
    greenYellowChtValue = yellowBorder;
    yellowRedChtValue = redBorder;
    minEgtValue = minEgt;
    maxEgtValue = maxEgt;
}

void ChtEgt::changeFlashState()
{
    if (QTime::currentTime().second() % 2 == 0) {
        flashState  = true;
    } else {
        flashState = false;
    }
}

void ChtEgt::onAlarmAck() {
    isAcknowledged = true;
}
