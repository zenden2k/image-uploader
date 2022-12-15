#ifndef IU_CORE_UTILS_IOEXCEPTION
#define IU_CORE_UTILS_IOEXCEPTION

#pragma once 

#include <string>

class IOException : public std::exception
{
public:
    /** Constructor (C strings).
     *  @param message C-style string error message.
     *                 The string contents are copied upon construction.
     *                 Hence, responsibility for deleting the char* lies
     *                 with the caller.
     */
    explicit IOException(const char* message)
        : msg_(message) {}

    /** Constructor (C++ STL strings).
     *  @param message The error message.
     */
    explicit IOException(const std::string& message)
        : msg_(message) {}

    explicit IOException(const std::string& message, const std::string& filename)
        : msg_(message), filename_(filename) {}

    /** Destructor.
     * Virtual to allow for subclassing.
     */
    ~IOException() noexcept override {}

    /** Returns a pointer to the (constant) error description.
     *  @return A pointer to a const char*. The underlying memory
     *          is in posession of the Exception object. Callers must
     *          not attempt to free the memory.
     */
    const char* what() const noexcept override {
        return msg_.c_str();
    }

protected:
    /** Error message.
     */
    std::string msg_, filename_;
};

#endif