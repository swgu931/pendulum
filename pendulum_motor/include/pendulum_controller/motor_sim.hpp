#include <chrono>
#include <cmath>
#include <atomic>

#include "motor.h"

#ifndef GRAVITY
#define GRAVITY 9.80665
#endif

#ifndef PI
#define PI 3.14159265359
#endif

namespace pendulum {


  /// Struct representing the physical properties of the pendulum.
  struct PendulumProperties
  {
    // Mass of the weight on the end of the pendulum in kilograms
    double mass = 0.01;
    // Length of the pendulum in meters
    double length = 0.5;
  };

  /// Struct representing the dynamic/kinematic state of the pendulum.
  struct PendulumState
  {
    // Angle from the ground in radians
    double position = 0;
    // Angular velocity in radians/sec
    double velocity = 0;
    // Angular acceleration in radians/sec^2
    double acceleration = 0;
    // Torque on the joint (currently unused)
    double torque = 0;
  };

class MotorSim : public Motor
{
public:

    MotorSim(std::chrono::nanoseconds period)
    : publish_period_(period)
    {
        // Calculate the controller timestep (for discrete differentiation/integration).
        dt_ = publish_period_.count() / (1000.0 * 1000.0 * 1000.0);
        if (std::isnan(dt_) || dt_ == 0) {
            throw std::runtime_error("Invalid dt_ calculated in PendulumController constructor");
        }
    }

    virtual void update_motor_command(const pendulum_msgs::msg::JointCommand &msg) override
    {
        // Assume direct, instantaneous position control
        // (It would be more realistic to simulate a motor model)
        state_.position = msg.position;

        // Enforce position limits
        if (state_.position > PI) {
          state_.position = PI;
        } else if (state_.position < 0) {
          state_.position = 0;
        }
    }

    virtual void update_motor_state()
    {
      state_.acceleration = GRAVITY * std::sin(state_.position - PI / 2.0) / properties_.length +
        state_.torque / (properties_.mass * properties_.length * properties_.length);
      state_.velocity += state_.acceleration * dt_;
      state_.position += state_.velocity * dt_;
      if (state_.position > PI) {
        state_.position = PI;
      } else if (state_.position < 0) {
        state_.position = 0;
      }
    }

    virtual float get_position() const
    {
      return state_.position;
    }
    virtual float get_velocity() const
    {
      return state_.velocity;
    }

private:
    std::chrono::nanoseconds publish_period_;
    double dt_;
    PendulumProperties properties_;
    PendulumState state_;
};


} // namespace pendulum
