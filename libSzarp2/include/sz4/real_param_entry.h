/* 
  SZARP: SCADA software 
  

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/
#ifndef __SZ4_REAL_PARAM_ENTRY_H__
#define __SZ4_REAL_PARAM_ENTRY_H__

namespace sz4 {

#if 0
class execution_engine : public ExecutionEngine {
	base* m_base;
	std::vector<double> m_vars;
public:
	execution_engine(base* _base, TParam* param) : m_base(_base) {
		LuaExec::Param* exec_param = param->GetLuaExecParam();
		m_vars.resize(exec_param->m_vars.size());

		m_vals[3] = PT_MIN10;
		m_vals[4] = PT_HOUR;
		m_vals[5] = PT_HOUR8;
		m_vals[6] = PT_DAY;
		m_vals[7] = PT_WEEK;
		m_vals[8] = PT_MONTH;
		m_vals[9] = PT_SEC10;

	}

	
#if 0
	void calculate_value(
		m_vals[0] = nan("");
		m_vals[1] = t;
		m_vals[2] = probe_type;
#endif

	virtual double Value(size_t param_index, const double& time_, const double& period_type) {
		ParamRef& ref = m_param->m_par_refs[param_index];
		//XXX
		weighted_sum<double, time_t> sum;
		m_base->get_weighted_sum(ref.m_param, start, szb_move_time(time_, 1, period_type, sum);
		if (sum.data_weight())
			return sum.sum() / sum.data_weight();
		else
			return nan("");
	}

	virtual std::vector<double>& Vars() {
		return m_vars;
	}
	~execution_engine() {}
}
#endif

template<class V, class T> class real_param_entry_in_buffer : public SzbParamObserver {
	base *m_base;
	typedef std::map<T, block_entry<V, T>*> map_type;
	map_type m_blocks;

	TParam* m_param;
	const boost::filesystem::wpath m_param_dir;
	std::vector<std::string> m_paths_to_update;

	boost::mutex m_mutex;

	bool m_refresh_file_list;
public:
	real_param_entry_in_buffer(base *_base, TParam* param, const boost::filesystem::wpath& param_dir) : m_base(_base), m_param(param), m_param_dir(param_dir), m_refresh_file_list(true)
		{}
	void get_weighted_sum_impl(const T& start, const T& end, SZARP_PROBE_TYPE, weighted_sum<V, T>& sum)  {
		refresh_if_needed();
		//search for the first block that has starting time bigger than
		//start_time
		if (m_blocks.size() == 0) {
			//no such block - nodata
			sum.no_data_weight() += end - start;
			return;
		}

		T current(start);
		typename map_type::iterator i = m_blocks.upper_bound(start);
		//if not first - back one off
		if (i != m_blocks.begin())
			std::advance(i, -1);
		do {
			block_entry<V, T>* entry = i->second;

			if (end < entry->block().start_time())
				break;

			if (current < entry->block().start_time()) {
				sum.no_data_weight() += entry->block().start_time() - current;
				current = i->first;
			}

			entry->refresh_if_needed();
			if (current < entry->block().end_time()) {
				T end_for_block = std::min(end, entry->block().end_time());
				entry->get_weighted_sum(current, end_for_block, sum);

				current = end_for_block;
			}

			std::advance(i, 1);
		} while (i != m_blocks.end());

		if (current < end)
			sum.no_data_weight() += end - current;

	}

	T search_data_right_impl(const T& start, const T& end, SZARP_PROBE_TYPE, const search_condition& condition) {
		refresh_if_needed();
		if (m_blocks.size() == 0)
			return invalid_time_value<T>::value;

		typename map_type::iterator i = m_blocks.upper_bound(start);
		//if not first - step one back
		if (i != m_blocks.begin())
			std::advance(i, -1);

		while (i != m_blocks.end()) {
			block_entry<V, T>* entry = i->second;
			if (entry->block().start_time() >= end)
				break;

			entry->refresh_if_needed();
			T t = entry->search_data_right(start, end, condition);
			if (invalid_time_value<T>::is_valid(t))
				return t;

			std::advance(i, 1);
		}

		return invalid_time_value<T>::value;
	}

	T search_data_left_impl(const T& start, const T& end, SZARP_PROBE_TYPE, const search_condition& condition) {
		refresh_if_needed();
		if (m_blocks.size() == 0)
			return invalid_time_value<T>::value;

		typename map_type::iterator i = m_blocks.upper_bound(start);
		if (i != m_blocks.begin())
			std::advance(i, -1);

		while (true) {
			block_entry<V, T>* entry = i->second;

			entry->refresh_if_needed();
			if (entry->block().end_time() <= end)
				break;

			T t = entry->search_data_left(start, end, condition);
			if (invalid_time_value<T>::is_valid(t))
				return t;

			if (i == m_blocks.begin())
				break;
			std::advance(i, -1);
		}

		return invalid_time_value<T>::value;
	}

	void refresh_if_needed() {
		{
			boost::mutex::scoped_lock lock(m_mutex);
			for (std::vector<std::string>::iterator i = m_paths_to_update.begin(); i != m_paths_to_update.end(); i++) {
				refresh_file(*i);
			}
			m_paths_to_update.clear();
		}
		if (m_refresh_file_list) {
			try {
				refresh_file_list();
			} catch (boost::filesystem::wfilesystem_error&) {}
			m_refresh_file_list = false;
		}
	}

	
	void refresh_file(const std::string& path) {
		T time = path_to_date<T>(path);
		if (time == invalid_time_value<T>::value)
			return;

		typename map_type::iterator i = m_blocks.find(time);
		if (i != m_blocks.end())
			i->second->set_needs_refresh();
		else
			m_refresh_file_list = true;
	}

	void refresh_file_list() {
		namespace fs = boost::filesystem;
		
		for (fs::wdirectory_iterator i(m_param_dir); 
				i != fs::wdirectory_iterator(); 
				i++) {

			std::wstring file_path = i->path().file_string();

			T file_time = path_to_date<T>(file_path);
			if (!invalid_time_value<T>::is_valid(file_time))
				continue;

			if (m_blocks.find(file_time) != m_blocks.end())
				continue;

			m_blocks.insert(std::make_pair(file_time, new block_entry<V, T>(file_time, file_path)));
		}
	}

	void register_at_monitor(SzbParamMonitor* monitor) {
		monitor->add_observer(this, m_param, m_param_dir.external_file_string(), 0);
	}

	void deregister_from_monitor(SzbParamMonitor* monitor) {
		monitor->remove_observer(this);
	}

	void param_data_changed(TParam*, const std::string& path) {
		boost::mutex::scoped_lock lock(m_mutex);
		m_paths_to_update.push_back(path);
	}

	~real_param_entry_in_buffer() {
		for (typename map_type::iterator i = m_blocks.begin(); i != m_blocks.end(); i++)
			delete i->second;
	}
};

}
#endif