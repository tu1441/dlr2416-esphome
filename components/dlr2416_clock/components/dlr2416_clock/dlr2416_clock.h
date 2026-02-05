#pragma once
#include "esphome/core/component.h"
#include "esphome/components/time/real_time_clock.h"
#include <string>

namespace esphome {
namespace dlr2416_clock {

class DLR2416Clock : public Component {
 public:
  void set_time(time::RealTimeClock *t) { time_ = t; }

  // setters generated from python
  void set_d0_pin(int v){d0_=v;} void set_d1_pin(int v){d1_=v;} void set_d2_pin(int v){d2_=v;}
  void set_d3_pin(int v){d3_=v;} void set_d4_pin(int v){d4_=v;} void set_d5_pin(int v){d5_=v;}
  void set_d6_pin(int v){d6_=v;}
  void set_a0_pin(int v){a0_=v;} void set_a1_pin(int v){a1_=v;} void set_wr_pin(int v){wr_=v;}
  void set_ce1_1_pin(int v){ce1_1_=v;} void set_ce2_1_pin(int v){ce2_1_=v;}
  void set_ce1_2_pin(int v){ce1_2_=v;} void set_ce2_2_pin(int v){ce2_2_=v;}

  void set_reverse(bool v){reverse_=v;}
  void set_clock_hold_ms(uint32_t v){clock_hold_ms_=v;}
  void set_scroll_step_ms(uint32_t v){scroll_step_ms_=v;}
  void set_slide_step_ms(uint32_t v){slide_step_ms_=v;}
  void set_msg_passes(int v){msg_passes_=v;}
  void set_message(const std::string &s){message_=s;}

  void setup() override {
    // outputs
    const int pins[] = {d0_,d1_,d2_,d3_,d4_,d5_,d6_,a0_,a1_,wr_,ce1_1_,ce2_1_,ce1_2_,ce2_2_};
    for (int p : pins) pinMode(p, OUTPUT);

    digitalWrite(wr_, HIGH);
    disable_all_();

    clear8_();
    last_tick_ms_ = millis();
    mode_ = MODE_CLOCK_HOLD;
    mode_started_ms_ = millis();
    last_anim_ms_ = 0;
  }

  void loop() override {
    tick_time_();

    const uint32_t now = millis();

    if (mode_ == MODE_CLOCK_HOLD) {
      char clk[9]; make_clock_(clk);
      show8_(clk);
      if (now - mode_started_ms_ >= clock_hold_ms_) {
        mode_ = MODE_SLIDE_OUT;
        mode_started_ms_ = now;
      }
      return;
    }

    if (mode_ == MODE_SLIDE_OUT) {
      char clk[9]; make_clock_(clk);
      slide_out_left_(clk);
      scroll_idx_ = 0;
      passes_done_ = 0;
      last_anim_ms_ = millis();
      mode_ = MODE_SCROLL_MSG;
      mode_started_ms_ = millis();
      return;
    }

    if (mode_ == MODE_SCROLL_MSG) {
      if (now - last_anim_ms_ < scroll_step_ms_) return;
      last_anim_ms_ += scroll_step_ms_;

      if (message_.empty()) message_ = "        ";
      const int L = (int) message_.size();

      char win[9];
      for (int i=0;i<8;i++) win[i] = message_[(scroll_idx_ + i) % L];
      win[8] = 0;
      show8_(win);

      scroll_idx_++;
      if (scroll_idx_ >= L) {
        scroll_idx_ = 0;
        passes_done_++;
        if (passes_done_ >= msg_passes_) {
          mode_ = MODE_SLIDE_IN;
          mode_started_ms_ = millis();
        }
      }
      return;
    }

    if (mode_ == MODE_SLIDE_IN) {
      char clk[9]; make_clock_(clk);
      slide_in_from_right_(clk);
      mode_ = MODE_CLOCK_HOLD;
      mode_started_ms_ = millis();
      return;
    }
  }

 private:
  // pins
  int d0_{-1},d1_{-1},d2_{-1},d3_{-1},d4_{-1},d5_{-1},d6_{-1};
  int a0_{-1},a1_{-1},wr_{-1};
  int ce1_1_{-1},ce2_1_{-1},ce1_2_{-1},ce2_2_{-1};

  // behavior
  bool reverse_{true};
  uint32_t clock_hold_ms_{10000}, scroll_step_ms_{160}, slide_step_ms_{70};
  int msg_passes_{5};
  std::string message_{"   KAHMA LUTTI VENTIIL   "};

  // time
  time::RealTimeClock *time_{nullptr};
  int hh_{12}, mm_{0}, ss_{0};
  uint32_t last_tick_ms_{0};

  // state machine
  enum Mode { MODE_CLOCK_HOLD, MODE_SLIDE_OUT, MODE_SCROLL_MSG, MODE_SLIDE_IN };
  Mode mode_{MODE_CLOCK_HOLD};
  uint32_t mode_started_ms_{0};
  uint32_t last_anim_ms_{0};
  int scroll_idx_{0};
  int passes_done_{0};

  void tick_time_() {
    // Prefer SNTP time if available; fallback to millis-based if not synced yet
    if (time_ != nullptr) {
      auto t = time_->now();
      if (t.is_valid()) {
        hh_ = t.hour; mm_ = t.minute; ss_ = t.second;
        return;
      }
    }
    const uint32_t now = millis();
    if (now - last_tick_ms_ >= 1000) {
      last_tick_ms_ += 1000;
      ss_++;
      if (ss_ >= 60) { ss_=0; mm_++; }
      if (mm_ >= 60) { mm_=0; hh_++; }
      if (hh_ >= 24) hh_=0;
    }
  }

  void set_bus_(uint8_t v) {
    digitalWrite(d0_, (v>>0)&1);
    digitalWrite(d1_, (v>>1)&1);
    digitalWrite(d2_, (v>>2)&1);
    digitalWrite(d3_, (v>>3)&1);
    digitalWrite(d4_, (v>>4)&1);
    digitalWrite(d5_, (v>>5)&1);
    digitalWrite(d6_, (v>>6)&1);
  }

  void disable_all_() {
    digitalWrite(ce1_1_, HIGH); digitalWrite(ce2_1_, HIGH);
    digitalWrite(ce1_2_, HIGH); digitalWrite(ce2_2_, HIGH);
  }

  void enable_(uint8_t which) {
    if (which == 0) { digitalWrite(ce1_1_, LOW); digitalWrite(ce2_1_, LOW); }
    else           { digitalWrite(ce1_2_, LOW); digitalWrite(ce2_2_, LOW); }
  }

  void write_char_on_(uint8_t which, uint8_t pos0to3, char ch) {
    uint8_t pos = pos0to3;
    if (reverse_) pos = 3 - pos;

    digitalWrite(a0_, (pos>>0)&1);
    digitalWrite(a1_, (pos>>1)&1);

    set_bus_((uint8_t)ch);   // 7-bit ASCII, bit7 ignored by DLR

    enable_(which);
    digitalWrite(wr_, LOW);
    delayMicroseconds(2);
    digitalWrite(wr_, HIGH);
    disable_all_();
  }

  void write8_(uint8_t gpos, char ch) {
    if (gpos < 4) write_char_on_(0, gpos, ch);
    else          write_char_on_(1, gpos-4, ch);
  }

  void show8_(const char s8[9]) {
    for (uint8_t i=0;i<8;i++) write8_(i, s8[i]);
  }

  void clear8_() { show8_("        "); }

  void make_clock_(char out[9]) {
    out[0] = char('0' + (hh_/10));
    out[1] = char('0' + (hh_%10));
    out[2] = ':';
    out[3] = char('0' + (mm_/10));
    out[4] = char('0' + (mm_%10));
    out[5] = ':';
    out[6] = char('0' + (ss_/10));
    out[7] = char('0' + (ss_%10));
    out[8] = 0;
  }

  void slide_out_left_(const char current[9]) {
    for (int step=0; step<=8; step++) {
      char buf[9];
      for (int i=0;i<8;i++) {
        int src = i + step;
        buf[i] = (src < 8) ? current[src] : ' ';
      }
      buf[8]=0;
      show8_(buf);
      delay(slide_step_ms_);
    }
  }

  void slide_in_from_right_(const char target[9]) {
    for (int step=8; step>=0; step--) {
      char buf[9];
      for (int i=0;i<8;i++) {
        int src = i - step;
        buf[i] = (src >= 0) ? target[src] : ' ';
      }
      buf[8]=0;
      show8_(buf);
      delay(slide_step_ms_);
    }
  }
};

}  // namespace dlr2416_clock
}  // namespace esphome
