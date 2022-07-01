#pragma once

#include "okapi/api/chassis/controller/chassisScales.hpp"
#include "pros/adi.hpp"
#include "tomobo/odometry/Odometry.hpp"
#include "tomobo/odometry/Pose.hpp"

#include <cmath>
#include <valarray>

namespace tomobo {
class EncoderOdometry : public Odometry {

public:
    EncoderOdometry(okapi::ChassisScales ichassisScales,
        pros::ADIEncoder iencoder_left, pros::ADIEncoder iencoder_right);

    EncoderOdometry(const EncoderOdometry& other);

    void step() override;

    std::int32_t maxTickDiff = 1000;

protected:
    pros::ADIEncoder encoder_left, encoder_right;
    okapi::ChassisScales chassisScales;

    std::valarray<std::int32_t> lastTicks{ encoder_left.get_value(), encoder_right.get_value()};
};
}
