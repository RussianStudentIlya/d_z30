#include "optimized_thread.h"

OptimizeThreadPool::OptimizeThreadPool() 
	: m_thread_count(thread::hardware_concurrency() != 0 ? thread::hardware_concurrency() : 4),
	m_thred_queues(m_thread_count)
{}

void OptimizeThreadPool::start()
{
	//������� ��������� ������� � �������� �� �  m_threads
	for (int i = 0; i <= m_thread_count; i++)
	{
		m_threads.emplace_back(&OptimizeThreadPool::threadFunc, this, i);
	}
}

void OptimizeThreadPool::stop()
{
	for (int i = 0; i < m_thread_count; i++)
	{
		task_type empty_task;
		m_thred_queues[i].push(empty_task);
	}

	for (auto& t : m_threads)
		t.join();
}

res_type OptimizeThreadPool::push_task(FuncType f, vector<int> arr, long l, long r)
{
	int queue_to_push = m_qindex++ % m_thread_count;//���������� ������ ������� � ������� ����� �������� ������
	task_type task = task_type([=] {f(arr, l, r); });

	res_type result = task.get_future();

	m_thred_queues[queue_to_push].push(task); // �������� � �������
	return result;

}

void OptimizeThreadPool::threadFunc(int qindex)
{
	//����������� ���� ������ ������ �� �������
	while (true)
	{
		task_type task_to_do;
		bool res; // ��� ���� ����� ���������� ������� �� �������� ������� �� �������
		int i = 0;
		for(; i < m_thread_count; i++) // �������� ������ �������� ������ �� �������
		{
			if (res = m_thred_queues[(qindex + i) % m_thread_count].fast_pop(task_to_do))
				break;
		}
		if(!res)
		{
			m_thred_queues[qindex].pop(task_to_do);
		}
		else if (!task_to_do.valid())
		{
			m_thred_queues[(qindex + i) % m_thread_count].push(task_to_do);
		}
		/// ���� ���-�� ������� ������ ���������
		if(!task_to_do.valid()) // ���� ������ � �������
		{
			return;
		}

		task_to_do();
	}
}

RequestHandler_2::RequestHandler_2()
{
	this->m_tpool.start();
}

RequestHandler_2::~RequestHandler_2()
{
	this->m_tpool.stop();
}

res_type RequestHandler_2::push_task(FuncType f, vector<int> arr, long l, long r)
{
	return res_type();
}

res_type RequestHandler_2::push_task(FuncType f, vector<int> arr, long l, long r)
{
	return this->m_tpool.push_task(f, arr, l, r);
}