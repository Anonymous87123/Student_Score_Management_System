#pragma once

#include "EduSys/common/Constants.hpp"
#include "EduSys/model/Score.hpp"
#include "EduSys/storage/BinaryRepository.hpp"

namespace EduSys {

class ScoreRepository : public BinaryRepository<Score> {
public:
    ScoreRepository() : BinaryRepository<Score>(SCORES_FILE) {}
};

} // namespace EduSys
