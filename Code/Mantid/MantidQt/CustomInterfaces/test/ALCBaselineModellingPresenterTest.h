#ifndef MANTIDQT_CUSTOMINTERFACES_ALCBASELINEMODELLINGTEST_H_
#define MANTIDQT_CUSTOMINTERFACES_ALCBASELINEMODELLINGTEST_H_

#include <cxxtest/TestSuite.h>
#include <gmock/gmock.h>

#include <boost/scoped_ptr.hpp>
#include <boost/assign/list_of.hpp>

#include "MantidAPI/FunctionFactory.h"
#include "MantidAPI/FrameworkManager.h"
#include "MantidAPI/WorkspaceFactory.h"

#include "MantidQtCustomInterfaces/Muon/ALCBaselineModellingPresenter.h"

using namespace MantidQt::CustomInterfaces;
using namespace testing;
using boost::scoped_ptr;

class MockALCBaselineModellingView : public IALCBaselineModellingView
{
public:
  void requestFit() { emit fitRequested(); }
  void requestAddSection() { emit addSectionRequested(); }
  void requestRemoveSection(size_t index) { emit removeSectionRequested(index); }

  void modifySectionSelector(size_t index, double min, double max)
  {
    emit sectionSelectorModified(index, min, max);
  }

  MOCK_METHOD0(initialize, void());

  MOCK_CONST_METHOD0(function, IFunction_const_sptr());
  MOCK_CONST_METHOD0(sections, std::vector<Section>());

  MOCK_METHOD1(setDataCurve, void(const QwtData&));
  MOCK_METHOD1(setCorrectedCurve, void(const QwtData&));
  MOCK_METHOD1(setBaselineCurve, void(const QwtData&));
  MOCK_METHOD1(setFunction, void(IFunction_const_sptr));

  MOCK_METHOD1(setSections, void(const std::vector<Section>&));
  MOCK_METHOD2(updateSection, void(size_t, Section));

  MOCK_METHOD1(setSectionSelectors, void(const std::vector<SectionSelector>&));
};

class MockALCBaselineModellingModel : public IALCBaselineModellingModel
{
public:
  MOCK_CONST_METHOD0(fittedFunction, IFunction_const_sptr());
  MOCK_CONST_METHOD0(correctedData, MatrixWorkspace_const_sptr());
  MOCK_CONST_METHOD0(data, MatrixWorkspace_const_sptr());

  MOCK_METHOD1(setData, void(MatrixWorkspace_const_sptr));
  MOCK_METHOD2(fit, void(IFunction_const_sptr, const std::vector<Section>&));
};

MATCHER_P(FunctionName, name, "") { return arg->name() == name; }

MATCHER_P3(FunctionParameter, param, value, delta, "")
{
  return fabs(arg->getParameter(param) - value) < delta;
}

MATCHER_P3(QwtDataX, i, value, delta, "") { return fabs(arg.x(i) - value) < delta; }
MATCHER_P3(QwtDataY, i, value, delta, "") { return fabs(arg.y(i) - value) < delta; }

class ALCBaselineModellingPresenterTest : public CxxTest::TestSuite
{
  MockALCBaselineModellingView* m_view;
  MockALCBaselineModellingModel* m_model;
  ALCBaselineModellingPresenter* m_presenter;

public:
  // This pair of boilerplate methods prevent the suite being created statically
  // This means the constructor isn't called when running otherl tests
  static ALCBaselineModellingPresenterTest *createSuite() { return new ALCBaselineModellingPresenterTest(); }
  static void destroySuite( ALCBaselineModellingPresenterTest *suite ) { delete suite; }

  ALCBaselineModellingPresenterTest()
  {
    FrameworkManager::Instance(); // To make sure everything is initialized
  }

  void setUp()
  {
    m_view = new NiceMock<MockALCBaselineModellingView>();
    m_model = new NiceMock<MockALCBaselineModellingModel>();
    m_presenter = new ALCBaselineModellingPresenter(m_view, m_model);
    m_presenter->initialize();
  }

  void tearDown()
  {
    delete m_presenter;
    delete m_model;
    delete m_view;
  }

  MatrixWorkspace_sptr createTestWs(size_t size, double deltaY = 0)
  {
    MatrixWorkspace_sptr ws = WorkspaceFactory::Instance().create("Workspace2D", 1, size, size);

    for (size_t i = 0; i < size; ++i)
    {
      ws->dataX(0)[i] = static_cast<double>(i + 1);
      ws->dataY(0)[i] = ws->dataX(0)[i] + deltaY;
      ws->dataE(0)[i] = 1;
    }

    return ws;
  }

  void test_initialize()
  {
    // Not using m_view and m_present, because they are already initialized after setUp()
    MockALCBaselineModellingView view;
    MockALCBaselineModellingModel model;
    ALCBaselineModellingPresenter presenter(&view, &model);
    EXPECT_CALL(view, initialize()).Times(1);
    presenter.initialize();
  }

  void test_setData()
  {
    MatrixWorkspace_const_sptr data = createTestWs(3, 1);
    EXPECT_CALL(*m_model, setData(data)).Times(1);

    EXPECT_CALL(*m_view, setDataCurve(AllOf(Property(&QwtData::size, 3),
                                            QwtDataX(0, 1, 1E-8), QwtDataX(2, 3, 1E-8),
                                            QwtDataY(0, 2, 1E-8), QwtDataY(2, 4, 1E-8))));

    m_presenter->setData(data);
  }

  void test_addSection()
  {
    EXPECT_CALL(*m_model, data()).WillRepeatedly(Return(createTestWs(10)));

    std::vector<IALCBaselineModellingView::Section> noSections;
    EXPECT_CALL(*m_view, sections()).WillRepeatedly(Return(noSections));

    EXPECT_CALL(*m_view, setSections(ElementsAre(Pair(1,10))));
    EXPECT_CALL(*m_view, setSectionSelectors(ElementsAre(Pair(1,10))));

    m_view->requestAddSection();
  }

  void test_removeSection()
  {
    std::vector<IALCBaselineModellingView::Section> sections;
    sections.push_back(std::make_pair(10,20));
    sections.push_back(std::make_pair(30,40));
    sections.push_back(std::make_pair(50,60));

    std::vector<IALCBaselineModellingView::SectionSelector> selectors;

    ON_CALL(*m_view, sections()).WillByDefault(ReturnPointee(&sections));
    EXPECT_CALL(*m_view, setSections(_)).WillRepeatedly(SaveArg<0>(&sections));
    EXPECT_CALL(*m_view, setSectionSelectors(_)).WillRepeatedly(SaveArg<0>(&selectors));

    m_view->requestRemoveSection(1);
    m_view->requestRemoveSection(0);

    TS_ASSERT_EQUALS(sections.size(), 1);
    TS_ASSERT_DELTA(sections[0].first, 50, 1E-8);
    TS_ASSERT_DELTA(sections[0].second, 60, 1E-8);

    TS_ASSERT_EQUALS(selectors.size(), 1);
    TS_ASSERT_DELTA(selectors[0].first, 50, 1E-8);
    TS_ASSERT_DELTA(selectors[0].second, 60, 1E-8);
  }

  void test_onSectionSelectorModified()

  {
    EXPECT_CALL(*m_view, updateSection(5, Pair(-2,3)));
    m_view->modifySectionSelector(5,-2,3);
  }

  void test_fit()
  {
    EXPECT_CALL(*m_view, function()).WillRepeatedly(
          Return(FunctionFactory::Instance().createFunction("FlatBackground")));

    std::vector<IALCBaselineModellingView::Section> sections;
    sections.push_back(std::make_pair(10,20));
    sections.push_back(std::make_pair(40,55));
    EXPECT_CALL(*m_view, sections()).WillRepeatedly(Return(sections));

    EXPECT_CALL(*m_view, function()).WillRepeatedly(
          Return(FunctionFactory::Instance().createFunction("FlatBackground")));

    EXPECT_CALL(*m_model, fit(AllOf(FunctionName("FlatBackground"),
                                    FunctionParameter("A0", 0, 1E-8)),
                              ElementsAre(Pair(10, 20),
                                          Pair(40, 55))));

    EXPECT_CALL(*m_model, fittedFunction()).WillRepeatedly(
          Return(FunctionFactory::Instance().createInitialized("name=FlatBackground, A0=5")));

    EXPECT_CALL(*m_view, setFunction(AllOf(FunctionName("FlatBackground"),
                                           FunctionParameter("A0", 5, 1E-8))));

    EXPECT_CALL(*m_model, data()).WillRepeatedly(Return(createTestWs(3, 999)));

    // Correct baseline curve set
    EXPECT_CALL(*m_view, setBaselineCurve(AllOf(Property(&QwtData::size, 3),
                                                QwtDataX(0, 1, 1E-8), QwtDataX(2, 3, 1E-8),
                                                QwtDataY(0, 5, 1E-8), QwtDataY(2, 5, 1E-8))));

    EXPECT_CALL(*m_model, correctedData()).WillRepeatedly(Return(createTestWs(3, 3)));

    // Correct corrected curve set
    EXPECT_CALL(*m_view, setCorrectedCurve(AllOf(Property(&QwtData::size, 3),
                                                 QwtDataX(0, 1, 1E-8), QwtDataX(2, 3, 1E-8),
                                                 QwtDataY(0, 4, 1E-8), QwtDataY(2, 6, 1E-8))));

    m_view->requestFit();
  }

};


#endif /* MANTIDQT_CUSTOMINTERFACES_ALCBASELINEMODELLINGTEST_H_ */
