#include "stats.hpp"
#include <boost/log/trivial.hpp>
#include <chrono>
#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/document/value.hpp>

namespace mwg {
stats::stats() {
    reset();
}

void stats::reset() {
    std::lock_guard<std::mutex> lk(mut);
    count = 0;
    countExceptions = 0;
    min = std::chrono::microseconds::max();
    max = std::chrono::microseconds::min();
    mean = fpmicros(0);
    m2 = fpmicros(0);
}

void stats::accumulate(const stats& addStats) {
    BOOST_LOG_TRIVIAL(fatal) << "stats::accumulate not implemented";
    exit(0);
}

void stats::record(std::chrono::microseconds dur) {
    std::lock_guard<std::mutex> lk(mut);
    count++;
    if (dur < min)
        min = dur;
    if (dur > max)
        max = dur;

    // This is an implementation of the following algorithm:
    // http://en.wikipedia.org/wiki/Algorithms_for_calculating_variance#Online_algorithm
    // and largely copied from src/mongo/db/pipeline/accumulator_std_dev.cpp
    const auto delta = dur - mean;
    mean += delta / count;
    // note that delta was computed with mean before it was updated.
    m2 += delta.count() * (dur - mean);
}

bsoncxx::document::value stats::getStats(bool withReset) {
    bsoncxx::builder::stream::document document{};
    if (count > 0) {
        document << "count" << getCount();
        if (count > 1) {
            document << "min" << getMin().count();
            document << "max" << getMax().count();
            if (count > 2) {
                document << "popStdDev" << getPopStdDev().count();
            }
        }
        document << "mean" << getMean().count();
    }
    if (countExceptions > 0) {
        document << "countExceptions" << getCountExceptions();
    }
    if (withReset)
        reset();
    return (document << bsoncxx::builder::stream::finalize);
}
}
