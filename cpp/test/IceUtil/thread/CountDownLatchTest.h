//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef COUNT_DOWN_LATCH_TEST_H
#define COUNT_DOWN_LATCH_TEST_H

#include <TestBase.h>

class CountDownLatchTest final : public TestBase
{
public:

    CountDownLatchTest();

private:

    void run() final;
};

#endif
