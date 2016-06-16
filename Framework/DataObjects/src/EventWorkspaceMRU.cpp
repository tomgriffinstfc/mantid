#include "MantidDataObjects/EventWorkspaceMRU.h"
#include "MantidKernel/System.h"

namespace Mantid {
namespace DataObjects {

//----------------------------------------------------------------------------------------------
/** Constructor
 */
EventWorkspaceMRU::EventWorkspaceMRU() {}

//----------------------------------------------------------------------------------------------
/** Destructor
 */
EventWorkspaceMRU::~EventWorkspaceMRU() {
  // Make sure you free up the memory in the MRUs
  for (auto &data : m_bufferedDataY) {
    if (data) {
      data->clear();
      delete data;
    }
  }

  for (auto &data : m_bufferedDataE) {
    if (data) {
      data->clear();
      delete data;
    }
  }

  for (auto &marker : m_markersToDeleteY) {
    delete marker;
  }
  for (auto &marker : m_markersToDeleteE) {
    delete marker;
  }
}

//---------------------------------------------------------------------------
/** This function makes sure that there are enough data
 * buffers (MRU's) for E for the number of threads requested.
 * @param thread_num :: thread number that wants a MRU buffer
 */
void EventWorkspaceMRU::ensureEnoughBuffersE(size_t thread_num) const {
  std::lock_guard<std::mutex> _lock(m_changeMruListsMutexE);
  if (m_bufferedDataE.size() <= thread_num) {
    m_bufferedDataE.resize(thread_num + 1, nullptr);
    for (auto &data : m_bufferedDataE) {
      if (!data)
        data = new mru_listE(50); // Create a MRU list with this many entries.
    }
  }
}
//---------------------------------------------------------------------------
/** This function makes sure that there are enough data
 * buffers (MRU's) for Y for the number of threads requested.
 * @param thread_num :: thread number that wants a MRU buffer
 */
void EventWorkspaceMRU::ensureEnoughBuffersY(size_t thread_num) const {
  std::lock_guard<std::mutex> _lock(m_changeMruListsMutexY);
  if (m_bufferedDataY.size() <= thread_num) {
    m_bufferedDataY.resize(thread_num + 1, nullptr);
    for (auto &data : m_bufferedDataY) {
      if (!data)
        data = new mru_listY(50); // Create a MRU list with this many entries.
    }
  }
}

//---------------------------------------------------------------------------
/// Clear all the data in the MRU buffers
void EventWorkspaceMRU::clear() {
  std::lock_guard<std::mutex> _lock(this->m_toDeleteMutex);

  // FIXME: don't clear the locked ones!
  for (auto &marker : m_markersToDeleteY)
    if (!marker->m_locked)
      delete marker;
  m_markersToDeleteY.clear();
  for (auto &marker : m_markersToDeleteE)
    if (!marker->m_locked)
      delete marker;
  m_markersToDeleteE.clear();

  // Make sure you free up the memory in the MRUs
  for (auto &data : m_bufferedDataY)
    if (data) {
      data->clear();
    };

  for (auto &data : m_bufferedDataE)
    if (data) {
      data->clear();
    };
}

//---------------------------------------------------------------------------
/** Find a Y histogram in the MRU
 *
 * @param thread_num :: number of the thread in which this is run
 * @param index :: index of the data to return
 * @return pointer to the TypeWithMarker that has the data; NULL if not found.
 */
HistogramData::Counts EventWorkspaceMRU::findY(size_t thread_num,
                                               size_t index) {
  std::lock_guard<std::mutex> _lock(m_changeMruListsMutexY);
  auto result = m_bufferedDataY[thread_num]->find(index);
  if (result)
    return result->m_data;
  return HistogramData::Counts();
}

/** Find a Y histogram in the MRU
 *
 * @param thread_num :: number of the thread in which this is run
 * @param index :: index of the data to return
 * @return pointer to the TypeWithMarker that has the data; NULL if not found.
 */
HistogramData::CountStandardDeviations
EventWorkspaceMRU::findE(size_t thread_num, size_t index) {
  std::lock_guard<std::mutex> _lock(m_changeMruListsMutexE);
  auto result = m_bufferedDataE[thread_num]->find(index);
  if (result)
    return result->m_data;
  return HistogramData::CountStandardDeviations();
}

/** Insert a new histogram into the MRU
 *
 * @param thread_num :: thread being accessed
 * @param data :: the new data
 * @param index :: index of the data to insert
 * @param locked :: locking status of the data
 */
void EventWorkspaceMRU::insertY(size_t thread_num, HistogramData::Counts data,
                                const size_t index, bool &locked) {
  std::lock_guard<std::mutex> _lock(m_changeMruListsMutexY);
  auto yWithMarker = new TypeWithMarker<HistogramData::Counts>(index, locked);
  yWithMarker->m_data = std::move(data);
  auto oldData = m_bufferedDataY[thread_num]->insert(yWithMarker);
  // And clear up the memory of the old one, if it is dropping out.
  if (oldData) {
    if (oldData->m_locked) {
      std::lock_guard<std::mutex> _lock(this->m_toDeleteMutex);
      m_markersToDeleteY.push_back(oldData);
    } else
      delete oldData;
  }
}

/** Insert a new histogram into the MRU
 *
 * @param thread_num :: thread being accessed
 * @param data :: the new data
 * @param index :: index of the data to insert
 * @param locked :: locking status of the data
 */
void EventWorkspaceMRU::insertE(size_t thread_num,
                                HistogramData::CountStandardDeviations data,
                                const size_t index, bool &locked) {
  std::lock_guard<std::mutex> _lock(m_changeMruListsMutexE);
  auto eWithMarker =
      new TypeWithMarker<HistogramData::CountStandardDeviations>(index, locked);
  eWithMarker->m_data = std::move(data);
  auto oldData = m_bufferedDataE[thread_num]->insert(eWithMarker);
  // And clear up the memory of the old one, if it is dropping out.
  if (oldData) {
    if (oldData->m_locked) {
      std::lock_guard<std::mutex> _lock(this->m_toDeleteMutex);
      m_markersToDeleteE.push_back(oldData);
    } else
      delete oldData;
  }
}

/** Delete any entries in the MRU at the given index
 *
 * @param index :: index to delete.
 */
void EventWorkspaceMRU::deleteIndex(size_t index) {
  std::lock_guard<std::mutex> _lock1(m_changeMruListsMutexE);
  for (auto &data : m_bufferedDataE)
    if (data)
      data->deleteIndex(index);
  std::lock_guard<std::mutex> _lock2(m_changeMruListsMutexY);
  for (auto &data : m_bufferedDataY)
    if (data)
      data->deleteIndex(index);
}

} // namespace Mantid
} // namespace DataObjects
