#pragma once
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include "../pgm/Program.h"
#include "../pgm/Parameter.h"

// Describes which element within the program a widget is bound to
enum class ElementKind { PAD, LAYER, ENVELOPE, FILTER1, FILTER2, MIXER, SLIDER };

struct ElementRef {
    ElementKind kind  = ElementKind::PAD;
    int pad_index     = 0;
    int sub_index     = 0;   // layer index (0-3) or slider index (0-1)
};

// Helpers: access elements by ElementRef (safe — all ultimately ref prog.buffer())
int         elem_get_int   (Program& prog, const ElementRef& ref, const Parameter& p);
void        elem_set_int   (Program& prog, const ElementRef& ref, const Parameter& p, int v);
std::string elem_get_string(Program& prog, const ElementRef& ref, const Parameter& p);
void        elem_set_string(Program& prog, const ElementRef& ref, const Parameter& p, const std::string& s);
double      elem_get_tuning(Program& prog, const ElementRef& ref);
void        elem_set_tuning(Program& prog, const ElementRef& ref, double v);
Range       elem_get_range (Program& prog, const ElementRef& ref, const Parameter& p);
void        elem_set_range (Program& prog, const ElementRef& ref, const Parameter& p, const Range& r);

// ---------------------------------------------------------------------------
// Base class for all parameter widgets
// ---------------------------------------------------------------------------
class ParamWidgetBase : public QWidget {
    Q_OBJECT
public:
    explicit ParamWidgetBase(QWidget* parent = nullptr) : QWidget(parent) {}
    virtual void set_pad(int pad_index) = 0;
    virtual void load() = 0;
};

// ---------------------------------------------------------------------------
// Int widget — decrement/value/increment for INT, OFF_INT
// ---------------------------------------------------------------------------
class IntParamWidget : public ParamWidgetBase {
    Q_OBJECT
public:
    IntParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent = nullptr);
    void set_pad(int pad_index) override;
    void load() override;
private:
    Program*   prog_;
    ElementRef ref_;
    Parameter  param_;
    QPushButton* dec_;
    QLineEdit*   display_;
    QPushButton* inc_;
};

// ---------------------------------------------------------------------------
// Combo widget — QComboBox for ENUM
// ---------------------------------------------------------------------------
class ComboParamWidget : public ParamWidgetBase {
    Q_OBJECT
public:
    ComboParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent = nullptr);
    void set_pad(int pad_index) override;
    void load() override;
private:
    Program*   prog_;
    ElementRef ref_;
    Parameter  param_;
    QComboBox* combo_;
};

// ---------------------------------------------------------------------------
// String widget — QLineEdit (read-only display of sample name)
// ---------------------------------------------------------------------------
class StringParamWidget : public ParamWidgetBase {
    Q_OBJECT
public:
    StringParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent = nullptr);
    void set_pad(int pad_index) override;
    void load() override;
private:
    Program*   prog_;
    ElementRef ref_;
    Parameter  param_;
    QLineEdit* edit_;
};

// ---------------------------------------------------------------------------
// Tuning widget — decrement/value/increment for TUNING (-36.0 to +36.0)
// ---------------------------------------------------------------------------
class TuningParamWidget : public ParamWidgetBase {
    Q_OBJECT
public:
    TuningParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent = nullptr);
    void set_pad(int pad_index) override;
    void load() override;
private:
    Program*        prog_;
    ElementRef      ref_;
    Parameter       param_;
    QPushButton*    dec_;
    QLineEdit*      display_;
    QPushButton*    inc_;
};

// ---------------------------------------------------------------------------
// Range widget — two decrement/value/increment groups for low/high RANGE
// ---------------------------------------------------------------------------
class RangeParamWidget : public ParamWidgetBase {
    Q_OBJECT
public:
    RangeParamWidget(Program* prog, ElementRef ref, const Parameter& p, QWidget* parent = nullptr);
    void set_pad(int pad_index) override;
    void load() override;
private:
    Program*  prog_;
    ElementRef ref_;
    Parameter param_;
    QPushButton* low_dec_;
    QLineEdit*   low_;
    QPushButton* low_inc_;
    QPushButton* high_dec_;
    QLineEdit*   high_;
    QPushButton* high_inc_;
};

// ---------------------------------------------------------------------------
// Factory: create the right widget for a given parameter type
// ---------------------------------------------------------------------------
ParamWidgetBase* make_param_widget(Program* prog, const ElementRef& ref,
                                   const Parameter& p, QWidget* parent = nullptr);
