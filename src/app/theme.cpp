#include <math.h>

#include <QApplication>
#include <QFile>
#include <QBitmap>
#include <QFontDatabase>
#include <QIcon>
#include <QObject>
#include <QPainter>
#include <QPair>
#include <QPixmap>
#include <QRegularExpression>
#include <QTextStream>
#include <QTransform>

#include "app/theme.hpp"

const QFont Theme::font_10 = QFont("Montserrat", 10);
const QFont Theme::font_12 = QFont("Montserrat", 12);
const QFont Theme::font_14 = QFont("Montserrat", 14);
const QFont Theme::font_16 = QFont("Montserrat", 16);
const QFont Theme::font_18 = QFont("Montserrat", 18);
const QFont Theme::font_24 = QFont("Montserrat", 24);
const QFont Theme::font_36 = QFont("Montserrat", 36);

const QSize Theme::icon_16 = QSize(16, 16);
const QSize Theme::icon_24 = QSize(24, 24);
const QSize Theme::icon_26 = QSize(26, 26);
const QSize Theme::icon_28 = QSize(28, 28);
const QSize Theme::icon_32 = QSize(32, 32);
const QSize Theme::icon_36 = QSize(36, 36);
const QSize Theme::icon_42 = QSize(42, 42);
const QSize Theme::icon_48 = QSize(48, 48);
const QSize Theme::icon_56 = QSize(56, 56);
const QSize Theme::icon_84 = QSize(84, 84);
const QSize Theme::icon_96 = QSize(96, 96);

const QColor Theme::danger_color = QColor(211, 47, 47);
const QColor Theme::success_color = QColor(56, 142, 60);

Theme::Theme() : QObject(qApp), palette(), color("azure")
{
    QFontDatabase::addApplicationFont(":/fonts/Titillium_Web/TitilliumWeb-Regular.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Montserrat/Montserrat-LightItalic.ttf");
    QFontDatabase::addApplicationFont(":/fonts/Montserrat/Montserrat-Regular.ttf");

    this->stylesheets["light"] = this->parse_stylesheet(":/stylesheets/light.qss");
    this->stylesheets["dark"] = this->parse_stylesheet(":/stylesheets/dark.qss");

    qApp->setStyleSheet(this->stylesheets[this->mode ? "dark" : "light"]);

    this->set_palette();
}

QString Theme::parse_stylesheet(QString file)
{
    QFile f(file);
    f.open(QFile::ReadOnly | QFile::Text);
    QTextStream s(&f);
    QString stylesheet(s.readAll());
    f.close();

    return stylesheet;
}

QString Theme::scale_stylesheet(QString stylesheet)
{
    QRegularExpression px_regex(" (-?\\d+)px");
    QRegularExpressionMatchIterator i = px_regex.globalMatch(stylesheet);
    while (i.hasNext()) {
        QRegularExpressionMatch match = i.next();
        if (match.hasMatch()) {
            int scaled_px = std::ceil(match.captured(1).toInt() * this->scale);
            stylesheet.replace(match.captured(), QString("%1px").arg(scaled_px));
        }
    }

    return stylesheet;
}

void Theme::set_palette()
{
    QColor new_color(this->colors[this->mode ? "dark" : "light"][this->color]);
    this->palette.setColor(QPalette::Base, new_color);
    new_color.setAlphaF(.5);
    this->palette.setColor(QPalette::AlternateBase, new_color);
    qApp->setPalette(this->palette);
}

QPixmap Theme::create_pixmap_variant(QPixmap &base, qreal opacity)
{
    QPixmap image(base.size());
    image.fill(Qt::transparent);

    QPainter painter(&image);
    painter.setOpacity(opacity);
    painter.drawPixmap(0, 0, base);
    painter.end();

    return image;
}

QIcon Theme::recolor_icon(QIcon icon, bool checkable)
{
    QPixmap base = icon.pixmap(512);
    QPixmap image(base.size());
    QColor c = this->get_color(this->color);
    c.setAlpha(this->mode ? 222 : 255);
    image.fill(c);
    image.setMask(base.createMaskFromColor(Qt::transparent));

    QPixmap image2(base.size());
    if (checkable)
        image2.fill(this->mode ? QColor(255, 255, 255, 102) : QColor(0, 0, 0, 178));
    else
        image2.fill(this->mode ? QColor(255, 255, 255, 222) : QColor(0, 0, 0, 255));
    image2.setMask(base.createMaskFromColor(Qt::transparent));

    QIcon icon2;
    icon2.addPixmap(image2, QIcon::Normal, QIcon::Off);
    icon2.addPixmap(image, QIcon::Normal, QIcon::On);

    return icon2;
}

void Theme::add_tab_icon(QString name, QWidget *widget, Qt::Orientation orientation)
{
    QTransform t;
    t.rotate((orientation == Qt::Orientation::Horizontal) ? 0 : 90);

    QPixmap dark_base = QIcon(QString(":/icons/dark/%1.svg").arg(name)).pixmap(512, 512).transformed(t);
    QPixmap dark_active = this->create_pixmap_variant(dark_base, .87);
    QPixmap dark_normal = this->create_pixmap_variant(dark_base, .54);
    QPixmap dark_disabled = this->create_pixmap_variant(dark_base, .38);

    QIcon dark_icon = QIcon(dark_normal);
    dark_icon.addPixmap(dark_active, QIcon::Active, QIcon::On);
    dark_icon.addPixmap(dark_disabled, QIcon::Disabled);
    this->tab_icons["dark"].append({widget, dark_icon});

    QPixmap light_base = QIcon(QString(":/icons/light/%1.svg").arg(name)).pixmap(512, 512).transformed(t);
    QPixmap light_active = this->create_pixmap_variant(light_base, 1);
    QPixmap light_normal = this->create_pixmap_variant(light_base, .7);
    QPixmap light_disabled = this->create_pixmap_variant(light_base, .5);

    QIcon light_icon = QIcon(light_normal);
    light_icon.addPixmap(light_active, QIcon::Active, QIcon::On);
    light_icon.addPixmap(light_disabled, QIcon::Disabled);
    this->tab_icons["light"].append({widget, light_icon});

    this->update();
}

void Theme::add_button_icon(QString name, QPushButton *button, QString normal_name)
{
    bool set_down_state = button->isCheckable() && button->text().isNull();

    QPixmap dark_base = QIcon(QString(":/icons/dark/%1.svg").arg(name)).pixmap(512, 512);
    QPixmap light_base = QIcon(QString(":/icons/light/%1.svg").arg(name)).pixmap(512, 512);

    QPixmap dark_active = this->create_pixmap_variant(dark_base, .87);
    QPixmap dark_disabled = this->create_pixmap_variant(dark_base, .38);

    QPixmap light_active = this->create_pixmap_variant(light_base, 1);
    QPixmap light_disabled = this->create_pixmap_variant(light_base, .5);

    QPixmap dark_normal;
    QPixmap light_normal;
    if (normal_name.isNull()) {
        dark_normal = set_down_state ? this->create_pixmap_variant(dark_base, .54) : dark_active;
        light_normal = set_down_state ? this->create_pixmap_variant(light_base, .7) : light_active;
    }
    else {
        QPixmap dark_normal_base = QIcon(QString(":/icons/dark/%1.svg").arg(normal_name)).pixmap(512, 512);
        QPixmap light_normal_base = QIcon(QString(":/icons/light/%1.svg").arg(normal_name)).pixmap(512, 512);

        dark_normal = this->create_pixmap_variant(dark_normal_base, .87);
        light_normal = this->create_pixmap_variant(light_normal_base, 1);
    }

    QIcon dark_icon = QIcon(dark_normal);
    dark_icon.addPixmap(dark_active, QIcon::Active, QIcon::On);
    dark_icon.addPixmap(dark_disabled, QIcon::Disabled);
    this->button_icons["dark"].append({button, dark_icon, button->iconSize()});

    QIcon light_icon = QIcon(light_normal);
    light_icon.addPixmap(light_active, QIcon::Active, QIcon::On);
    light_icon.addPixmap(light_disabled, QIcon::Disabled);
    this->button_icons["light"].append({button, light_icon, button->iconSize()});

    this->update();
}

QIcon Theme::add_button_icon2(QString name, QPushButton *button, QString normal_name)
{
    return this->recolor_icon(QIcon(QString(":/icons/%1.svg").arg(name)), button->isCheckable());
}

void Theme::update()
{
    this->set_palette();
    qApp->setStyleSheet(this->scale_stylesheet(this->stylesheets[this->mode ? "dark" : "light"]));

    for (QWidget *widget : qApp->allWidgets()) {
        QFont font = widget->font();
        font.setPointSize(std::ceil(font.pointSize() * this->scale));
        widget->setFont(font);

        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button != nullptr && !button->icon().isNull()) {
            button->setIcon(this->recolor_icon(button->icon(), button->property("page").isValid()));
            QVariant base_icon_size = button->property("base_icon_size");
            if (base_icon_size.isValid()) {
                QSize size = base_icon_size.value<QSize>();
                size.rwidth() *= this->scale;
                size.rheight() *= this->scale;
                button->setIconSize(size);
            }
        }
    }

    emit mode_updated(this->mode);
    emit icons_updated(this->tab_icons[this->mode ? "dark" : "light"],
                       this->button_icons[this->mode ? "dark" : "light"], this->scale);
    emit color_updated();
}

Theme *Theme::get_instance()
{
    static Theme theme;
    return &theme;
}
