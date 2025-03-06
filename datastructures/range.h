#ifndef RANGE_H
#define RANGE_H

#include <string>
#include <sstream>

/**
 * @brief A class representing a range of double values with upper and lower bounds
 */
class Range {
private:
    double lowerBound;
    double upperBound;

public:
    /**
     * @brief Default constructor, creates an empty range [0, 0]
     */
    Range() : lowerBound(0.0), upperBound(0.0) {}

    /**
     * @brief Constructor with specified bounds
     * @param lowerBound The lower bound of the range
     * @param upperBound The upper bound of the range
     */
    Range(double lowerBound, double upperBound) 
        : lowerBound(lowerBound), upperBound(upperBound) {}

    /**
     * @brief Get the lower bound of the range
     * @return The lower bound as a double
     */
    double getLowerBound() const { return lowerBound; }

    /**
     * @brief Get the upper bound of the range
     * @return The upper bound as a double
     */
    double getUpperBound() const { return upperBound; }

    /**
     * @brief Set the lower bound of the range
     * @param lowerBound The new lower bound value
     */
    void setLowerBound(double lowerBound) { lowerBound = lowerBound; }

    /**
     * @brief Set the upper bound of the range
     * @param upperBound The new upper bound value
     */
    void setUpperBound(double upperBound) { upperBound = upperBound; }

    /**
     * @brief Check if a value is within this range (inclusive)
     * @param value The value to check
     * @return true if the value is within the range, false otherwise
     */
    bool contains(double value) const {
        return value >= lowerBound && value <= upperBound;
    }

     /**
     * @brief Check if another range completely falls inside this range
     * @param other The other range to check
     * @return true if the other range is completely contained within this range, false otherwise
     */
     bool contains(const Range& other) const {
        return other.getLowerBound() >= lowerBound && other.getUpperBound() <= upperBound;
    }

    /**
     * @brief Get a string representation of the range
     * @return A string in the format "[lowerBound, upperBound]"
     */
    std::string toString() const {
        std::ostringstream oss;
        oss << "[" << lowerBound << ", " << upperBound << "]";
        return oss.str();
    }

    /**
     * @brief Get the width/span of the range
     * @return The difference between upper and lower bounds
     */
    double getWidth() const {
        return upperBound - lowerBound;
    }

    /**
    * @brief Check if the range has zero width (lower bound equals upper bound)
    * @param epsilon Optional tolerance value for floating-point comparison
    * @return true if the range has zero width, false otherwise
    */
    bool isPoint(double epsilon = 1e-4) const {
        return std::abs(upperBound - lowerBound) < epsilon;
    }
};

#endif // RANGE_H