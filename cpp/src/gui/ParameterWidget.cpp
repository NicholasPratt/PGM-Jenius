#include "ParameterWidget.h"
#include "../pgm/Pad.h"
#include "../pgm/Layer.h"
#include "../pgm/PadEnvelope.h"
#include "../pgm/PadFilter.h"
#include "../pgm/PadMixer.h"
#include "../pgm/Slider.h"
#include <QApplication>
#include <QFormLayout>
#include <algorithm>
#include <stdexcept>

// ---------------------------------------------------------------------------
// Element access helpers
// ---------------------------------------------------------------------------

static BaseElement make_element(Program& prog, const ElementRef& ref, Pad& pad_out) {
    // Helper to build the right element — pad_out must outlive the returned element
    // This is a conceptual helper; we call per-type directly below.
    (void)prog; (void)ref; (void)pad_out;
    throw std::logic_error("use per-type accessors");
}

namespace {
QPushButton* make_step_button(const QString& text, QWidget* parent) {
    auto* button = new QPushButton(text, parent);
    QFont font = button->font();
    font.setPointSize(font.pointSize() + 1);
    font.setBold(true);
    button->setFont(font);
    button->setFixedSize(24, 24);
    button->setText(text);
    button->setStyleSheet(
        "QPushButton {"
        "  padding: 0px;"
        "  margin: 0px;"
        "  border-radius: 5px;"
        "  font-weight: 700;"
        "}"
    );
    button->setAutoRepeat(true);
    button->setAutoRepeatDelay(250);
    button->setAutoRepeatInterval(80);
    return button;
}

QLineEdit* make_value_box(int min_width, QWidget* parent) {
    auto* edit = new QLineEdit(parent);
    edit->setAlignment(Qt::AlignCenter);
    edit->setMinimumWidth(min_width);
    edit->setMaximumWidth(min_width + 8);
    edit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    return edit;
}

QString format_int_value(const Parameter& parameter, int value) {
    const bool is_pan = parameter.get_label() == "Pan"
                     && parameter.get_range().low == 0
                     && parameter.get_range().high == 100;
    if (is_pan) {
        if (value <= 0) return "L50";
        if (value < 50) return QString("L%1").arg(50 - value);
        if (value == 50) return "MID";
        if (value < 100) return QString("R%1").arg(value - 50);
        return "R50";
    }
    if (parameter.get_type() == ParamType::OFF_INT && value == 0)
        return "Off";
    return QString::number(value);
}

QString format_tuning_value(double value) {
    return QString::number(value, 'f', 2);
}

bool parse_int_value(const Parameter& parameter, const QString& text, int* value_out) {
    const QString trimmed = text.trimmed();
    const bool is_pan = parameter.get_label() == "Pan"
                     && parameter.get_range().low == 0
                     && parameter.get_range().high == 100;
    if (is_pan) {
        if (trimmed.compare("MID", Qt::CaseInsensitive) == 0) {
            *value_out = 50;
            return true;
        }
        if (trimmed.size() >= 2) {
            const QChar side = trimmed.at(0).toUpper();
            bool ok = false;
            const int amount = trimmed.mid(1).toInt(&ok);
            if (ok && amount >= 1 && amount <= 50) {
                if (side == 'L') {
                    *value_out = 50 - amount;
                    return true;
                }
                if (side == 'R') {
                    *value_out = 50 + amount;
                    return true;
                }
            }
        }
    }
    if (parameter.get_type() == ParamType::OFF_INT && trimmed.compare("off", Qt::CaseInsensitive) == 0) {
        *value_out = 0;
        return true;
    }

    bool ok = false;
    const int parsed = trimmed.toInt(&ok);
    if (!ok) return false;
    *value_out = parsed;
    return true;
}

void update_combo_checks(QComboBox* combo, int current_index) {
    for (int i = 0; i < combo->count(); ++i) {
        combo->setItemData(
            i,
            i == current_index ? Qt::Checked : Qt::Unchecked,
            Qt::CheckStateRole
        );
    }
}
}

int elem_get_int(Program& prog, const ElementRef& ref, const Parameter& p) {
    switch (ref.kind) {
        case ElementKind::PAD: {
            Pad pad = prog.get_pad(ref.pad_index);
            return pad.get(p);
        }
        case ElementKind::LAYER: {
            Pad pad = prog.get_pad(ref.pad_index);
            Layer l = pad.get_layer(ref.sub_index);
            return l.get(p);
        }
        case ElementKind::ENVELOPE: {
            Pad pad = prog.get_pad(ref.pad_index);
            PadEnvelope e = pad.get_envelope();
            return e.get(p);
        }
        case ElementKind::FILTER1: {
            Pad pad = prog.get_pad(ref.pad_index);
            PadFilter1 f = pad.get_filter1();
            return f.get(p);
        }
        case ElementKind::FILTER2: {
            Pad pad = prog.get_pad(ref.pad_index);
            PadFilter2 f = pad.get_filter2();
            return f.get(p);
        }
        case ElementKind::MIXER: {
            Pad pad = prog.get_pad(ref.pad_index);
            PadMixer m = pad.get_mixer();
            return m.get(p);
        }
        case ElementKind::SLIDER: {
            Slider s = prog.get_slider(ref.sub_index);
            return s.get(p);
        }
    }
    return 0;
}

void elem_set_int(Program& prog, const ElementRef& ref, const Parameter& p, int v) {
    switch (ref.kind) {
        case ElementKind::PAD: {
            Pad pad = prog.get_pad(ref.pad_index);
            pad.set(p, v); return;
        }
        case ElementKind::LAYER: {
            Pad pad = prog.get_pad(ref.pad_index);
            Layer l = pad.get_layer(ref.sub_index);
            l.set(p, v); return;
        }
        case ElementKind::ENVELOPE: {
            Pad pad = prog.get_pad(ref.pad_index);
            PadEnvelope e = pad.get_envelope();
            e.set(p, v); return;
        }
        case ElementKind::FILTER1: {
            Pad pad = prog.get_pad(ref.pad_index);
            PadFilter1 f = pad.get_filter1();
            f.set(p, v); return;
        }
        case ElementKind::FILTER2: {
            Pad pad = prog.get_pad(ref.pad_index);
            PadFilter2 f = pad.get_filter2();
            f.set(p, v); return;
        }
        case ElementKind::MIXER: {
            Pad pad = prog.get_pad(ref.pad_index);
            PadMixer m = pad.get_mixer();
            m.set(p, v); return;
        }
        case ElementKind::SLIDER: {
            Slider s = prog.get_slider(ref.sub_index);
            s.set(p, v); return;
        }
    }
}

std::string elem_get_string(Program& prog, const ElementRef& ref, const Parameter&) {
    // Only layers have string parameters (sample name)
    Pad pad = prog.get_pad(ref.pad_index);
    Layer l = pad.get_layer(ref.sub_index);
    return l.get_sample_name();
}

void elem_set_string(Program& prog, const ElementRef& ref, const Parameter&, const std::string& s) {
    Pad pad = prog.get_pad(ref.pad_index);
    Layer l = pad.get_layer(ref.sub_index);
    l.set_sample_name(s);
}

double elem_get_tuning(Program& prog, const ElementRef& ref) {
    Pad pad = prog.get_pad(ref.pad_index);
    Layer l = pad.get_layer(ref.sub_index);
    return l.get_tuning();
}

void elem_set_tuning(Program& prog, const ElementRef& ref, double v) {
    Pad pad = prog.get_pad(ref.pad_index);
    Layer l = pad.get_layer(ref.sub_index);
    l.set_tuning(v);
}

Range elem_get_range(Program& prog, const ElementRef& ref, const Parameter& p) {
    switch (ref.kind) {
        case ElementKind::LAYER: {
            Pad pad = prog.get_pad(ref.pad_index);
            Layer l = pad.get_layer(ref.sub_index);
            return l.get_range();
        }
        case ElementKind::SLIDER: {
            Slider s = prog.get_slider(ref.sub_index);
            return s.getRange(p.get_offset());
        }
        default:
            break;
    }
    return {};
}

void elem_set_range(Program& prog, const ElementRef& ref, const Parameter& p, const Range& r) {
    switch (ref.kind) {
        case ElementKind::LAYER: {
            Pad pad = prog.get_pad(ref.pad_index);
            Layer l = pad.get_layer(ref.sub_index);
            l.set_range(r);
            return;
        }
        case ElementKind::SLIDER: {
            Slider s = prog.get_slider(ref.sub_index);
            s.setRange(p.get_offset(), r);
            return;
        }
        default:
            return;
    }
}

// ---------------------------------------------------------------------------
// IntParamWidget
// ---------------------------------------------------------------------------
IntParamWidget::IntParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent)
    : ParamWidgetBase(parent), prog_(prog), ref_(ref), param_(p)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    auto* label = new QLabel(QString::fromStdString(p.get_label()) + ":", this);
    layout->addWidget(label);

    dec_ = make_step_button("-", this);
    display_ = make_value_box(44, this);
    inc_ = make_step_button("+", this);
    layout->addWidget(dec_);
    layout->addWidget(display_, 1);
    layout->addWidget(inc_);

    connect(dec_, &QPushButton::clicked, this, [this]() {
        const Range& range = param_.get_range();
        int value = elem_get_int(*prog_, ref_, param_);
        if (value > range.low) {
            elem_set_int(*prog_, ref_, param_, value - 1);
            load();
        }
    });
    connect(inc_, &QPushButton::clicked, this, [this]() {
        const Range& range = param_.get_range();
        int value = elem_get_int(*prog_, ref_, param_);
        if (value < range.high) {
            elem_set_int(*prog_, ref_, param_, value + 1);
            load();
        }
    });
    connect(display_, &QLineEdit::editingFinished, this, [this]() {
        const Range& range = param_.get_range();
        int value = 0;
        if (!parse_int_value(param_, display_->text(), &value) || value < range.low || value > range.high) {
            QApplication::beep();
            load();
            return;
        }

        elem_set_int(*prog_, ref_, param_, value);
        load();
    });
    load();
}

void IntParamWidget::set_pad(int pad_index) {
    ref_.pad_index = pad_index;
    load();
}

void IntParamWidget::load() {
    const int value = elem_get_int(*prog_, ref_, param_);
    display_->blockSignals(true);
    display_->setText(format_int_value(param_, value));
    display_->blockSignals(false);
}

// ---------------------------------------------------------------------------
// ComboParamWidget
// ---------------------------------------------------------------------------
ComboParamWidget::ComboParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent)
    : ParamWidgetBase(parent), prog_(prog), ref_(ref), param_(p)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    auto* label = new QLabel(QString::fromStdString(p.get_label()) + ":", this);
    layout->addWidget(label);

    combo_ = new QComboBox(this);
    combo_->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    combo_->setMinimumContentsLength(1);
    combo_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    for (const auto& val : p.get_enum_values())
        combo_->addItem(QString::fromStdString(val));
    layout->addWidget(combo_, 1);

    connect(combo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
        elem_set_int(*prog_, ref_, param_, idx);
        update_combo_checks(combo_, idx);
    });
    load();
}

void ComboParamWidget::set_pad(int pad_index) {
    ref_.pad_index = pad_index;
    load();
}

void ComboParamWidget::load() {
    const int current_index = elem_get_int(*prog_, ref_, param_);
    combo_->blockSignals(true);
    combo_->setCurrentIndex(current_index);
    combo_->blockSignals(false);
    update_combo_checks(combo_, current_index);
}

// ---------------------------------------------------------------------------
// StringParamWidget
// ---------------------------------------------------------------------------
StringParamWidget::StringParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent)
    : ParamWidgetBase(parent), prog_(prog), ref_(ref), param_(p)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    auto* label = new QLabel(QString::fromStdString(p.get_label()) + ":", this);
    layout->addWidget(label);

    edit_ = new QLineEdit(this);
    edit_->setMaxLength(16);
    edit_->setPlaceholderText("(empty)");
    edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(edit_, 1);

    connect(edit_, &QLineEdit::editingFinished, this, [this]() {
        elem_set_string(*prog_, ref_, param_, edit_->text().toStdString());
    });
    load();
}

void StringParamWidget::set_pad(int pad_index) {
    ref_.pad_index = pad_index;
    load();
}

void StringParamWidget::load() {
    edit_->blockSignals(true);
    edit_->setText(QString::fromStdString(elem_get_string(*prog_, ref_, param_)));
    edit_->blockSignals(false);
}

// ---------------------------------------------------------------------------
// TuningParamWidget
// ---------------------------------------------------------------------------
TuningParamWidget::TuningParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent)
    : ParamWidgetBase(parent), prog_(prog), ref_(ref), param_(p)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    auto* label = new QLabel(QString::fromStdString(p.get_label()) + ":", this);
    layout->addWidget(label);

    dec_ = make_step_button("-", this);
    display_ = make_value_box(54, this);
    inc_ = make_step_button("+", this);
    layout->addWidget(dec_);
    layout->addWidget(display_, 1);
    layout->addWidget(inc_);

    connect(dec_, &QPushButton::clicked, this, [this]() {
        const Range& range = param_.get_range();
        double value = elem_get_tuning(*prog_, ref_);
        value -= 0.1;
        if (value < range.low)
            value = range.low;
        elem_set_tuning(*prog_, ref_, value);
        load();
    });
    connect(inc_, &QPushButton::clicked, this, [this]() {
        const Range& range = param_.get_range();
        double value = elem_get_tuning(*prog_, ref_);
        value += 0.1;
        if (value > range.high)
            value = range.high;
        elem_set_tuning(*prog_, ref_, value);
        load();
    });
    connect(display_, &QLineEdit::editingFinished, this, [this]() {
        bool ok = false;
        const double value = display_->text().trimmed().toDouble(&ok);
        if (!ok || !param_.get_range().contains(value)) {
            QApplication::beep();
            load();
            return;
        }

        elem_set_tuning(*prog_, ref_, value);
        load();
    });
    load();
}

void TuningParamWidget::set_pad(int pad_index) {
    ref_.pad_index = pad_index;
    load();
}

void TuningParamWidget::load() {
    const double value = elem_get_tuning(*prog_, ref_);
    display_->blockSignals(true);
    display_->setText(format_tuning_value(value));
    display_->blockSignals(false);
}

// ---------------------------------------------------------------------------
// RangeParamWidget
// ---------------------------------------------------------------------------
RangeParamWidget::RangeParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent)
    : ParamWidgetBase(parent), prog_(prog), ref_(ref), param_(p)
{
    auto* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    auto* label = new QLabel(QString::fromStdString(p.get_label()) + ":", this);
    layout->addWidget(label);

    layout->addWidget(new QLabel("L", this));
    low_dec_ = make_step_button("-", this);
    low_ = make_value_box(32, this);
    low_inc_ = make_step_button("+", this);
    layout->addWidget(low_dec_);
    layout->addWidget(low_);
    layout->addWidget(low_inc_);

    layout->addWidget(new QLabel("H", this));
    high_dec_ = make_step_button("-", this);
    high_ = make_value_box(32, this);
    high_inc_ = make_step_button("+", this);
    layout->addWidget(high_dec_);
    layout->addWidget(high_);
    layout->addWidget(high_inc_);

    auto save_fn = [this](int low, int high) {
        if (low > high)
            std::swap(low, high);
        Range r(low, high);
        elem_set_range(*prog_, ref_, param_, r);
        load();
    };
    connect(low_dec_, &QPushButton::clicked, this, [this, save_fn]() {
        Range r = elem_get_range(*prog_, ref_, param_);
        if (r.low > param_.get_range().low)
            save_fn(r.low - 1, r.high);
    });
    connect(low_inc_, &QPushButton::clicked, this, [this, save_fn]() {
        Range r = elem_get_range(*prog_, ref_, param_);
        if (r.low < param_.get_range().high)
            save_fn(r.low + 1, r.high);
    });
    connect(high_dec_, &QPushButton::clicked, this, [this, save_fn]() {
        Range r = elem_get_range(*prog_, ref_, param_);
        if (r.high > param_.get_range().low)
            save_fn(r.low, r.high - 1);
    });
    connect(high_inc_, &QPushButton::clicked, this, [this, save_fn]() {
        Range r = elem_get_range(*prog_, ref_, param_);
        if (r.high < param_.get_range().high)
            save_fn(r.low, r.high + 1);
    });
    connect(low_, &QLineEdit::editingFinished, this, [this, save_fn]() {
        bool ok = false;
        const int low = low_->text().trimmed().toInt(&ok);
        Range current = elem_get_range(*prog_, ref_, param_);
        if (!ok || low < param_.get_range().low || low > param_.get_range().high) {
            QApplication::beep();
            load();
            return;
        }
        save_fn(low, current.high);
    });
    connect(high_, &QLineEdit::editingFinished, this, [this, save_fn]() {
        bool ok = false;
        const int high = high_->text().trimmed().toInt(&ok);
        Range current = elem_get_range(*prog_, ref_, param_);
        if (!ok || high < param_.get_range().low || high > param_.get_range().high) {
            QApplication::beep();
            load();
            return;
        }
        save_fn(current.low, high);
    });
    load();
}

void RangeParamWidget::set_pad(int pad_index) {
    ref_.pad_index = pad_index;
    load();
}

void RangeParamWidget::load() {
    Range r = elem_get_range(*prog_, ref_, param_);
    low_->blockSignals(true);
    high_->blockSignals(true);
    low_->setText(QString::number(r.low));
    high_->setText(QString::number(r.high));
    low_->blockSignals(false);
    high_->blockSignals(false);
}

// ---------------------------------------------------------------------------
// Factory
// ---------------------------------------------------------------------------
ParamWidgetBase* make_param_widget(Program* prog, const ElementRef& ref,
                                   const Parameter& p, QWidget* parent)
{
    switch (p.get_type()) {
        case ParamType::TEXT:    return new StringParamWidget(prog, ref, p, parent);
        case ParamType::TUNING:  return new TuningParamWidget(prog, ref, p, parent);
        case ParamType::RANGE:   return new RangeParamWidget (prog, ref, p, parent);
        case ParamType::ENUM:    return new ComboParamWidget  (prog, ref, p, parent);
        case ParamType::INT:
        case ParamType::OFF_INT: return new IntParamWidget   (prog, ref, p, parent);
    }
    return nullptr;
}
