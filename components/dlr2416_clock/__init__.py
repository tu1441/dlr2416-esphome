import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import time
from esphome.const import CONF_ID

dlr2416_clock_ns = cg.esphome_ns.namespace("dlr2416_clock")
DLR2416Clock = dlr2416_clock_ns.class_("DLR2416Clock", cg.Component)

CONF_TIME_ID = "time_id"
CONF_MESSAGE = "message"
CONF_MSG_PASSES = "msg_passes"
CONF_REVERSE = "reverse_pos_within_display"
CONF_CLOCK_HOLD_MS = "clock_hold_ms"
CONF_SCROLL_STEP_MS = "scroll_step_ms"
CONF_SLIDE_STEP_MS = "slide_step_ms"

PIN_KEYS = [
  "d0_pin","d1_pin","d2_pin","d3_pin","d4_pin","d5_pin","d6_pin",
  "a0_pin","a1_pin","wr_pin",
  "ce1_1_pin","ce2_1_pin","ce1_2_pin","ce2_2_pin"
]

CONFIG_SCHEMA = cv.Schema({
  cv.GenerateID(): cv.declare_id(DLR2416Clock),
  cv.Required(CONF_TIME_ID): cv.use_id(time.RealTimeClock),

  cv.Required("d0_pin"): cv.int_,
  cv.Required("d1_pin"): cv.int_,
  cv.Required("d2_pin"): cv.int_,
  cv.Required("d3_pin"): cv.int_,
  cv.Required("d4_pin"): cv.int_,
  cv.Required("d5_pin"): cv.int_,
  cv.Required("d6_pin"): cv.int_,

  cv.Required("a0_pin"): cv.int_,
  cv.Required("a1_pin"): cv.int_,
  cv.Required("wr_pin"): cv.int_,

  cv.Required("ce1_1_pin"): cv.int_,
  cv.Required("ce2_1_pin"): cv.int_,
  cv.Required("ce1_2_pin"): cv.int_,
  cv.Required("ce2_2_pin"): cv.int_,

  cv.Optional(CONF_REVERSE, default=True): cv.boolean,

  cv.Optional(CONF_CLOCK_HOLD_MS, default=10000): cv.int_,
  cv.Optional(CONF_SCROLL_STEP_MS, default=160): cv.int_,
  cv.Optional(CONF_SLIDE_STEP_MS, default=70): cv.int_,

  cv.Optional(CONF_MSG_PASSES, default=5): cv.int_,
  cv.Optional(CONF_MESSAGE, default="   KAHMA LUTTI VENTIIL   "): cv.string,
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
  var = cg.new_Pvariable(config[CONF_ID])
  await cg.register_component(var, config)

  t = await cg.get_variable(config[CONF_TIME_ID])
  cg.add(var.set_time(t))

  # Pins
  for k in PIN_KEYS:
    cg.add(getattr(var, f"set_{k}")(config[k]))

  cg.add(var.set_reverse(config[CONF_REVERSE]))
  cg.add(var.set_clock_hold_ms(config[CONF_CLOCK_HOLD_MS]))
  cg.add(var.set_scroll_step_ms(config[CONF_SCROLL_STEP_MS]))
  cg.add(var.set_slide_step_ms(config[CONF_SLIDE_STEP_MS]))
  cg.add(var.set_msg_passes(config[CONF_MSG_PASSES]))
  cg.add(var.set_message(config[CONF_MESSAGE]))
