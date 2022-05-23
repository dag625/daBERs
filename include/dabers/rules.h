//
// Created by Daniel Garcia on 5/22/2022.
//

#ifndef DABERS_RULES_H
#define DABERS_RULES_H

namespace dabers {

    struct ber{};
    struct cer{};
    struct der{};

    enum class rules : int {
        ber,
        cer,
        der
    };

} /* namespace dabers */

#endif //DABERS_RULES_H
