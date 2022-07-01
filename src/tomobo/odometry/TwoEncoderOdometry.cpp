#include "tomobo/odometry/TwoEncoderOdometry.hpp"
#include "tomobo/odometry/EncoderOdometry.hpp"
#include "okapi/api.hpp"

using namespace okapi::literals;
using namespace okapi;

namespace tomobo {

TwoEncoderOdometry::TwoEncoderOdometry(okapi::ChassisScales ichassisScales,
    pros::ADIEncoder iencoder_left, pros::ADIEncoder iencoder_right,
    pros::ADIEncoder iencoder_mid) :
    chassisScales(ichassisScales),
    encoder_left(iencoder_left),
    encoder_right(iencoder_right),
    encoder_mid(iencoder_mid),
    EncoderOdometry(ichassisScales, iencoder_left, iencoder_right){};

void TwoEncoderOdometry::step() {
    const auto deltaT = timer.getDt();

    if (deltaT.convert(millisecond) == 0) {
        return; // TODO: log
    }

    std::valarray<std::int32_t> newTicks = {
        encoder_left.get_value(), encoder_right.get_value()
    };

    std::valarray<int32_t> deltaTicks = newTicks - lastTicks;
    lastTicks = newTicks;

    // Make sure all tick values are possible
    for (std::int32_t i : deltaTicks) {
        if (abs(i) > maxTickDiff)
            return; //TODO: Log an error in these situtations
    }

    // Convert from ticks to meters
    const double deltaL = deltaTicks[0] / chassisScales.straight;
    const double deltaR = deltaTicks[1] / chassisScales.straight;

    // Calculate change in angle
    double deltaTheta = (deltaL - deltaR) / chassisScales.wheelTrack.convert(meter);

    // Calculate absolute angle
    //double newTheta = ((newTicks[0] / chassisScales.straight) - (newTicks[1] / chassisScales.straight))
    //    / chassisScales.wheelTrack.convert(meter);

    /* First take encoder ticks and turn into meters.
       Subtract the distance the wheel has travelled due to rotation, usinghe
       arc length formula.
    */
    const double deltaS = 0; // (deltaTicks[2] / chassisScales.middle)

    double locDX, locDY;

    // If we're going in a straight line, we don't need to use the arc approximation method
    if (deltaL == deltaR) {
        locDX = deltaS;
        locDY = deltaR;
    } else {
        // Calculate chord lengths
        locDX = deltaS;
        locDY = 2.0 * sin(deltaTheta / 2.0) * ((deltaR / deltaTheta) + (chassisScales.wheelTrack.convert(meter) / 2.0));
    }

    // angle to rotate back by
    double avgTheta = pose.theta.convert(radian) + (deltaTheta / 2.0);

    // Convert from Cartesian to Polar coordinates, rotating the coordinates by avgTheta in the process
    double radial = hypot(locDX, locDY);
    double angular = atan2(locDY, locDX) - avgTheta;

    // Convert back
    double dX = radial * cos(angular);
    double dY = radial * sin(angular);

    if (isnan(dX))
        dX = 0;
    if (isnan(dY))
        dY = 0;
    if (isnan(deltaTheta))
        deltaTheta = 0;

    // Update pose
    pose.x += meter * dX;
    pose.y += meter * dY;
    pose.theta += radian * deltaTheta;
};

}
