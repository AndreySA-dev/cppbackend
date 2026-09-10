#ifndef PERF_TIMER_H
#define PERF_TIMER_H

#include <chrono>
#include <iostream>

template <typename duration_t>
class perf_timer
{
	template <typename duration_t_>
	friend std::ostream &operator<<(std::ostream &output, const perf_timer<duration_t_> &timer);
public:
	perf_timer();
	perf_timer(perf_timer<duration_t>& ) = delete;
	perf_timer<duration_t> operator=(perf_timer<duration_t>&) = delete;
	void restart();
	void stop();
	void resume();
	int64_t get_duration() const;
private:
	std::chrono::steady_clock::time_point start_point;
	std::chrono::steady_clock::time_point end_point;
	duration_t duration;
	bool is_started;
};

template <typename duration_t>
perf_timer<duration_t>::perf_timer()
{
	restart();
}

template <typename duration_t>
void perf_timer<duration_t>::restart()
{
	start_point = end_point = std::chrono::steady_clock::now();
	duration = duration_t(0);
	is_started = true;
}

template <typename duration_t>
void perf_timer<duration_t>::stop()
{
	end_point = std::chrono::steady_clock::now();
	is_started = false;
	duration += std::chrono::duration_cast<duration_t>(end_point - start_point);
}

template <typename duration_t>
void perf_timer<duration_t>::resume()
{
	start_point = std::chrono::steady_clock::now();
	is_started = true;
}

template <typename duration_t>
int64_t perf_timer<duration_t>::get_duration() const
{
	if (is_started)
		return (
			duration + 
			std::chrono::duration_cast<duration_t>(
				std::chrono::steady_clock::now() - start_point
			)
		).count();
	else
		return duration.count();
}

template <typename duration_t>
std::ostream &operator<<(std::ostream &output, const perf_timer<duration_t> &timer)
{
		output << timer.get_duration();
		return output;
}

#endif