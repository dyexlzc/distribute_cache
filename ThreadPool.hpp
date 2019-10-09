#ifndef _THREADPOOL_H
#define _THREADPOOL_H
#include<mutex>
#include<thread>
#include<queue>
#include<condition_variable>
#include<string>
#include<iostream>
using namespace std;
class ThreadPool {
public:
	class job {
	public:
		virtual void run() { };
	};
private:
	queue<job*> _job;//任务列表
	mutex mutex_global, job_mutex;
	condition_variable cv;
	thread *tlist;
	int num_running, n;//任务个数
	bool stop_tok;
public:

	ThreadPool(int const &_n=0):n(_n) {
		//初始化线程池个数
		if (_n == 0) {
			this->n = std::move(this->core_num());
			stop_tok=false;
		}
		//cout << this->n;
		num_running = 0;
		tlist = new thread[n];
		for (int i = 0; i < n; i++) {
			tlist[i] = thread([this, i]() {//直接在这里用lambda表达式建立线程

				while (1) {
					unique_lock<mutex> lock(this->job_mutex);
					this->cv.wait(lock, [this] {
						return !this->_job.empty()||stop_tok;
					});//等待阻塞线程，直到update中通知状态改变
					if (this->_job.size() != 0)
					{
						this->mutex_global.lock();
						job* job_to_run = this->_job.front(); this->_job.pop();
						lock.unlock();
						this->num_running++;this->mutex_global.unlock();
						job_to_run->run();
						//delete job_to_run;
						this->mutex_global.lock(); this->num_running--;this->mutex_global.unlock();
					}
					if(stop_tok==true){
						return;
					}
				}
			});

		}
	}

	void addTask(job& job) {
		this->_job.push(&job);
		this->update();

	}
	void update() {
		//更新线程状态
		this->cv.notify_one();
	}
	void free(){
		stop_tok=true;
	}
	unsigned int core_num() {
		return std::thread::hardware_concurrency();
	}

};
#endif