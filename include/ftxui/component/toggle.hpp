#ifndef FTXUI_COMPONENT_TOGGLE_H_
#define FTXUI_COMPONENT_TOGGLE_H_

#include <functional>
#include <string>

#include "ftxui/component/component.hpp"

namespace ftxui {

class Toggle : public Component {
 public:
  // Constructor.
  ~Toggle() override = default;

  // State.
  int selected = 0;
  std::vector<std::wstring> entries = {L"On", L"Off"};

  Decorator focused_style = inverted;
  Decorator selected_style = bold;
  Decorator normal_style = dim;

  // Callback.
  std::function<void()> on_change = []() {};

  // Component implementation.
  Element Render() override;
  bool OnEvent(Event) override;
};

}  // namespace ftxui

#endif /* end of include guard: FTXUI_COMPONENT_TOGGLE_H_ */
