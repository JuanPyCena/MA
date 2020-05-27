///////////////////////////////////////////////////////////////////////////////
//
// Package:    AVCOMMON - Avibit Common Libraries
// QT-Version: QT4
// Copyright:  AviBit data processing GmbH, 2001-2011
//
// Module:     AVLIB - AviBit Library
//
/////////////////////////////////////////////////////////////////////////////

/*! \file
  \author  DI Alexander Wemmer, a.wemmer@avibit.com
  \author  QT4-PORT: Dr. Ralf Gamillscheg, r.gamillscheg@avibit.com
  \brief
*/



// QT includes
#include <QRect>
#include <QDataStream>
#include <QLocale>

#include "avlog.h"

#include "avdomhelper.h"

/////////////////////////////////////////////////////////////////////////////

QString AVDOMHelper::printNode(const QDomNode& node, int ident)
{
    QString out;
    QTextStream stream(&out, QIODevice::WriteOnly);
    node.save(stream, ident);
    return out;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseInt32AttributeFromElement(const QDomElement& element,
                                                 const QString& attribute_name,
                                                 qint32& value)
{
    AVASSERT(!element.isNull());
    AVASSERT(!attribute_name.isEmpty());
    value = 0;

    if (!element.hasAttribute(attribute_name))
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseInt32AttributeFromElement: "
                        "element (%s) has no attribute with name (%s)",
                        qPrintable(element.nodeName()), qPrintable(attribute_name));
        return false;
    }

    bool convok = false;
    value = element.attribute(attribute_name).toInt(&convok);
    if (!convok)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseInt32AttributeFromElement: "
                        "value of attribute (%s) of element (%s) could not be converted to INT32",
                        qPrintable(attribute_name), qPrintable(element.nodeName()));
        value = 0;
        return false;
    }

    AVLogger->Write(LOG_DEBUG1, "AVDOMHelper:parseInt32AttributeFromElement: "
                    "got element (%s) with attribute (%s) = (%d)",
                    qPrintable(element.nodeName()), qPrintable(attribute_name), value);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseStringAttributeFromElement(const QDomElement& element,
                                                  const QString& attribute_name,
                                                  QString& value)
{
    AVASSERT(!element.isNull());
    AVASSERT(!attribute_name.isEmpty());
    value = QString::null;

    if (!element.hasAttribute(attribute_name))
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseStringAttributeFromElement: "
                        "element (%s) has no attribute with name (%s)",
                        qPrintable(element.nodeName()), qPrintable(attribute_name));
        return false;
    }

    value = element.attribute(attribute_name);

    AVLogger->Write(LOG_DEBUG1, "AVDOMHelper:parseInt32AttributeFromElement: "
                    "got element (%s) with attribute (%s) = (%s)",
                    qPrintable(element.nodeName()), qPrintable(attribute_name), qPrintable(value));

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseDoubleAttributeFromElement(const QDomElement& element,
                                                  const QString& attribute_name,
                                                  double& value)
{
    AVASSERT(!element.isNull());
    AVASSERT(!attribute_name.isEmpty());
    value = 0.0;

    if (!element.hasAttribute(attribute_name))
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseDoubleAttributeFromElement: "
                        "element (%s) has no attribute with name (%s)",
                        qPrintable(element.nodeName()), qPrintable(attribute_name));
        return false;
    }

    bool convok = false;
    value = element.attribute(attribute_name).toDouble(&convok);
    if (!convok)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseDoubleAttributeFromElement: "
                        "value of attribute (%s) of element (%s) could not be converted to DOUBLE",
                        qPrintable(attribute_name), qPrintable(element.nodeName()));
        value = 0.0;
        return false;
    }

    AVLogger->Write(LOG_DEBUG1, "AVDOMHelper:parseInt32AttributeFromElement: "
                    "got element (%s) with attribute (%s) = (%f)",
                    qPrintable(element.nodeName()), qPrintable(attribute_name), value);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseInt32ContentFromElement(const QDomElement& element,
                                               qint32& value)
{
    AVASSERT(!element.isNull());
    value = 0;

    bool convok = false;
    value = element.text().toInt(&convok);
    if (!convok)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseInt32ContentFromElement: "
                        "content of element (%s) could not be converted to INT32",
                        qPrintable(element.nodeName()));
        value = 0;
        return false;
    }

    AVLogger->Write(LOG_DEBUG1, "AVDOMHelper:parseInt32ContentFromElement: "
                    "got element (%s) = (%d)",
                    qPrintable(element.nodeName()), value);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseUInt32ContentFromElement(const QDomElement& element,
                                                quint32& value)
{
    AVASSERT(!element.isNull());
    value = 0;

    bool convok = false;
    value = element.text().toUInt(&convok);
    if (!convok)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseUInt32ContentFromElement: "
                        "content of element (%s) could not be converted to INT32",
                        qPrintable(element.nodeName()));
        value = 0;
        return false;
    }

    AVLogger->Write(LOG_DEBUG1, "AVDOMHelper:parseUInt32ContentFromElement: "
                    "got element (%s) = (%d)",
                    qPrintable(element.nodeName()), value);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseBoolContentFromElement(const QDomElement& element,
                                              bool& value)
{
    AVASSERT(!element.isNull());
    value = 0;

    bool convok = false;
    uint tmpValue;
    tmpValue = element.text().toUInt(&convok);
    if (!convok || tmpValue > 1)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseBoolContentFromElement: "
                        "content of element (%s) could not be converted to bool",
                        qPrintable(element.nodeName()));
        value = 0;
        return false;
    }
    value = tmpValue;

    AVLogger->Write(LOG_DEBUG1, "AVDOMHelper:parseBoolContentFromElement: "
                    "got element (%s) = (%d)",
                    qPrintable(element.nodeName()), value);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseStringContentFromElement(const QDomElement& element,
                                                QString& value)
{
    AVASSERT(!element.isNull());
    value = QString::null;

    value = element.text();

    AVLogger->Write(LOG_DEBUG1, "AVDOMHelper:parseInt32ContentFromElement: "
                    "got element (%s) = (%s)",
                    qPrintable(element.nodeName()), qPrintable(value));

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseDoubleContentFromElement(const QDomElement& element,
                                                double& value)
{
    AVASSERT(!element.isNull());
    value = 0.0;

    bool convok = false;
    value = element.text().toDouble(&convok);
    if (!convok)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseInt32ContentFromElement: "
                        "content of element (%s) could not be converted to DOUBLE",
                        qPrintable(element.nodeName()));
        value = 0.0;
        return false;
    }

    AVLogger->Write(LOG_DEBUG1, "AVDOMHelper:parseInt32ContentFromElement: "
                    "got element (%s) = (%f)",
                    qPrintable(element.nodeName()), value);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseQRectContentFromElement(const QDomElement& element, QRect& value)
{

    AVASSERT(!element.isNull());

    QString attr;

    int right, bottom,
        left, top;

    bool convok = false;

    attr = element.attribute(ATTR_QRECT_RIGHT);
    if( attr.isEmpty() ) return false;

    right  = attr.toInt(&convok);

    attr = element.attribute(ATTR_QRECT_BOTTOM);
    if( attr.isEmpty() ) return false;

    bottom = attr.toInt(&convok);

    attr = element.attribute(ATTR_QRECT_LEFT);
    if( attr.isEmpty() ) return false;

    left   = attr.toInt(&convok);

    attr = element.attribute(ATTR_QRECT_TOP);
    if( attr.isEmpty() ) return false;

    top    = attr.toInt(&convok);

    if (!convok)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseQPointContentFromElement: "
                        "one or mor attributes of element (%s) could not be converted to Int",
                        qPrintable(element.nodeName()));
        return false;
    }

    value.setBottomRight( QPoint(right, bottom) );
    value.setTopLeft( QPoint(left , top ) );

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseQPointContentFromElement(const QDomElement& element, QPoint& value)
{
    AVASSERT(!element.isNull());

    QString attr;

    int x, y;
    bool convok = false;

    attr   = element.attribute(ATTR_QPOINT_X);
    if( attr.isEmpty() ) return false;

    x  = attr.toInt(&convok);

    attr = element.attribute(ATTR_QPOINT_Y);
    if( attr.isEmpty() ) return false;

    y = attr.toInt(&convok);

    if (!convok)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseQPointContentFromElement: "
                        "one or more attributes of element (%s) could not be converted to Int (%s)",
                        qPrintable(element.nodeName()), qPrintable(attr));
        return false;
    }

    value.setX(x);
    value.setY(y);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseQPointFContentFromElement(const QDomElement& element, QPointF& value)
{
    AVASSERT(!element.isNull());

    QString attr;

    double x, y;
    bool convok = true;

    attr   = element.attribute(ATTR_QPOINTF_X);
    if( attr.isEmpty() ) return false;

    convok &= AVFromString(attr, x);

    attr = element.attribute(ATTR_QPOINTF_Y);
    if( attr.isEmpty() ) return false;

    convok &= AVFromString(attr, y);

    if (!convok)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseQPointFContentFromElement: "
                        "one or more attributes of element (%s) could not be converted to Double (%s)",
                        qPrintable(element.nodeName()), qPrintable(attr));
        return false;
    }

    value.setX(x);
    value.setY(y);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseQSizeContentFromElement(const QDomElement& element, QSize& value)
{
    AVASSERT(!element.isNull());

    QString attr;

    int width, height;
    bool convok = false;

    attr   = element.attribute(ATTR_QSIZE_WIDTH);
    if( attr.isEmpty() ) return false;

    width  = attr.toInt(&convok);

    attr = element.attribute(ATTR_QSIZE_HEIGHT);
    if( attr.isEmpty() ) return false;

    height = attr.toInt(&convok);

    if (!convok)
    {
        AVLogger->Write(LOG_ERROR, "AVDOMHelper:parseQSizeContentFromElement: "
                        "one or more attributes of element (%s) could not be converted to Int",
                        qPrintable(element.nodeName()));
        return false;
    }

    value.setWidth(width);
    value.setHeight(height);

    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseQPolygonContentFromElement(const QDomElement& element,
                                                  QPolygon &value)
{
    AVASSERT(!element.isNull());
    value.clear();

    for(QDomElement point_element=element.firstChildElement(CHILD_QPOLYGON_POINT);
            !point_element.isNull();
            point_element=point_element.nextSiblingElement(CHILD_QPOLYGON_POINT))
    {
        QPoint point;
        bool convok=false;

        convok=parseQPointContentFromElement(point_element,point);

        if(!convok)
        {
            value.clear();

            AVLogger->Write(LOG_ERROR, "AVDOMHelper::parseQPolygonContentFromElement: "
                            "one or more points of QPolygon (%s) could not be converted",
                            qPrintable(element.nodeName()));
            return false;
        }
        value.append(point);
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

bool AVDOMHelper::parseQPointFPolygonContentFromElement(const QDomElement& element,
                                                        QVector<QPointF> &value)
{
    AVASSERT(!element.isNull());
    value.clear();

    for(QDomElement point_element=element.firstChildElement(CHILD_QPOINTFPOLYGON_POINT);
            !point_element.isNull();
            point_element=point_element.nextSiblingElement(CHILD_QPOINTFPOLYGON_POINT))
    {
        QPointF point;
        bool convok=false;

        convok=parseQPointFContentFromElement(point_element,point);

        if(!convok)
        {
            value.clear();

            AVLogger->Write(LOG_ERROR, "AVDOMHelper::parseQPointFPolygonContentFromElement: "
                            "one or more points of polygon (%s) could not be converted",
                            qPrintable(element.nodeName()));
            return false;
        }
        value.append(point);
    }
    return true;
}

/////////////////////////////////////////////////////////////////////////////

QDomElement AVDOMHelper::createBoolElement(QDomDocument &document,
                                           const QString &name, bool value)
{
    QDomElement element=document.createElement(name);
    QDomText text=document.createTextNode(value ? "1" : "0");
    element.appendChild(text);
    return element;
}

/////////////////////////////////////////////////////////////////////////////

QDomElement AVDOMHelper::createDoubleElement(QDomDocument &document,
                                               const QString &name, qreal value)
{
    QDomElement element=document.createElement(name);
    QDomText text=document.createTextNode(QString::number(value,'g',20));
    element.appendChild(text);
    return element;
}

/////////////////////////////////////////////////////////////////////////////

QDomElement AVDOMHelper::createQPointElement(
        QDomDocument &document,const QString &name, const QPoint &value)
{
    QDomElement element=document.createElement(name);
    element.setAttribute(ATTR_QPOINT_X,value.x());
    element.setAttribute(ATTR_QPOINT_Y,value.y());
    return element;
}

/////////////////////////////////////////////////////////////////////////////

QDomElement AVDOMHelper::createQPointFElement(
        QDomDocument &document,const QString &name, const QPointF &value)
{
    QDomElement element=document.createElement(name);
    element.setAttribute(ATTR_QPOINTF_X,QString::number(value.x(), 'f', 20));
    element.setAttribute(ATTR_QPOINTF_Y,QString::number(value.y(), 'f', 20));
    return element;
}

/////////////////////////////////////////////////////////////////////////////

QDomElement AVDOMHelper::createQPolygonElement(QDomDocument &document,const QString &name,
                                               const QPolygon &value)
{
    QDomElement element=document.createElement(name);
    Q_FOREACH(const QPoint &point,value)
    {
        QDomElement point_element=createQPointElement(document,CHILD_QPOLYGON_POINT,point);
        element.appendChild(point_element);
    }
    return element;
}

/////////////////////////////////////////////////////////////////////////////

QDomElement AVDOMHelper::createQPointFPolygonElement(QDomDocument &document,const QString &name,
                                                     const QVector<QPointF> &value)
{
    QDomElement element=document.createElement(name);
    Q_FOREACH(const QPointF &point,value)
    {
        QDomElement point_element=createQPointFElement(document,CHILD_QPOINTFPOLYGON_POINT,point);
        element.appendChild(point_element);
    }
    return element;
}

/////////////////////////////////////////////////////////////////////////////

void AVDOMHelper::createDoubleAttribute(QDomElement &element,const QString &name,
                                               double value)
{
    element.setAttribute(name,QString::number(value,'g',20));
}

/////////////////////////////////////////////////////////////////////////////


// End of file
