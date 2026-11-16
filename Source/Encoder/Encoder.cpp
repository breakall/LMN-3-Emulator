#include "Encoder.h"

Encoder::Encoder() {
    setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    setRange(0.0, 127.0, 1.0);
    setValue(64.0, juce::dontSendNotification);
    setVelocityBasedMode(false);
    setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    setRotaryParameters(0, juce::MathConstants<float>::twoPi, false);
}

Encoder::~Encoder() {}

void Encoder::addListener(Encoder::Listener *l) { listeners.add(l); }

void Encoder::removeListener(Encoder::Listener *l) { listeners.remove(l); }

void Encoder::mouseDoubleClick(const juce::MouseEvent &) {
    listeners.call([this](Listener &l) { l.encoderDoubleClicked(this); });
}

void Encoder::mouseUp(const juce::MouseEvent &event) {
    juce::Slider::mouseUp(event);
    // Re-centre after dragging so the next gesture can turn either way.
    setValue(64.0, juce::dontSendNotification);
    previousValue = 64;
}

void Encoder::mouseWheelMove(const juce::MouseEvent &event,
                             const juce::MouseWheelDetails &wheel) {
    modifierKeysChanged(event.mods);
    const float delta = wheel.deltaY != 0 ? wheel.deltaY : wheel.deltaX;
    if (delta == 0)
        return;
    const int steps =
        juce::jlimit(1, 16, juce::roundToInt(std::abs(delta) * 10));
    for (int i = 0; i < steps; ++i) {
        const int next = (int(getValue()) + (delta > 0 ? 1 : 127)) % 128;
        setValue(next, juce::sendNotificationSync);
    }
}

void Encoder::valueChanged() {
    // Edge cases whenever we roll over from the larger end of values to the
    // smaller end or vice versa. Also the difference between values is not
    // guaranteed to be equal to interval Depending on mouse speed and other
    // factors, it is equal to (minimum + (interval * an integer)). Therefore,
    // we simply check to see if the change between previous and current value
    // is greater than half of the entire range
    double range = getRange().getEnd() - getRange().getStart();
    if (std::abs(getValue() - previousValue) > (range / 2.0)) {
        if (getValue() > previousValue) {
            listeners.call([this](Listener &l) { l.encoderDecreased(this); });
        } else {
            listeners.call([this](Listener &l) { l.encoderIncreased(this); });
        }
    } else {
        if (getValue() > previousValue) {
            listeners.call([this](Listener &l) { l.encoderIncreased(this); });
        } else {
            listeners.call([this](Listener &l) { l.encoderDecreased(this); });
        }
    }
    previousValue = getValue();
}

void Encoder::modifierKeysChanged(const juce::ModifierKeys &modifiers) {
    // This is necessary to keep the slider from capturing (and not passing up)
    // the ctrl key events
    if (getParentComponent() != nullptr)
        getParentComponent()->modifierKeysChanged(modifiers);
}
