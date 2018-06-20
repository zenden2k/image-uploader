#include <gtest/gtest.h>

#include "../FontEnumerator.h"

class FontEnumeratorTest : public ::testing::Test {
public:
    void enumerationFinished() {
        enumerationFinishedCalled = true;
    }

    bool containsFont(const std::vector<LOGFONT>&fonts, LPCTSTR fontName) {
        for (const auto& font : fonts) {
            if (!lstrcmpi(fontName, font.lfFaceName)) {
                return true;
            }
        }
        return false;
    }
    bool enumerationFinishedCalled = false;
};

TEST_F(FontEnumeratorTest, Run)
{
    std::vector<LOGFONT> fonts;
    HDC dc = GetDC(0);
    FontEnumerator fe(dc, fonts, FontEnumerator::OnEnumerationFinishedDelegate(this, &FontEnumeratorTest::enumerationFinished));
    fe.run();
    ReleaseDC(0, dc);
    EXPECT_TRUE(fonts.size() > 0);
    EXPECT_TRUE(enumerationFinishedCalled);
    EXPECT_TRUE(containsFont(fonts, _T("Arial")));
    EXPECT_TRUE(containsFont(fonts, _T("Times New Roman")));
    EXPECT_TRUE(containsFont(fonts, _T("Courier New")));
    EXPECT_FALSE(containsFont(fonts, _T("vjo0fidjvudifvoffg")));
}