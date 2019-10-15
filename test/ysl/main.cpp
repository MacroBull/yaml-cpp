
#include <cmath>
#include <map>
#include <thread>
#include <vector>

#include <yaml-cpp/contrib/ysl.hpp>
#include <yaml-cpp/stlemitter.h>

int main()
{
	// setup glog
	FLAGS_logtostderr = true;
	FLAGS_minloglevel = 0;
	FLAGS_v           = 2;

	// setup YSL for current thread
	YAML::StreamLogger::set_thread_format(YAML::LoggerFormat::Indent, 4);

	// plain glog LOG
	LOG(INFO) << "Hello world!";

	// simple key-value scope
	{
		YSL(INFO) << YAML::ThreadFrame("Hello YSL") << YAML::BeginMap;
		YSL(INFO) << "name"
				  << "Mark McGwire";
		YSL(INFO) << "hr" << 65;
		YSL(INFO) << "avg" << 0.278f;
		YSL(INFO) << YAML::EndMap;
	}

	// stl and flow emission, explicit end-of-document
	{
		YSL(INFO) << YAML::ThreadFrame("Hello Container");
		YSL(INFO) << YAML::FloatPrecision(3);
		YSL(INFO) << YAML::BeginMap;
		YSL(INFO) << "hello" << std::map<int, float>{{1, 3.4f}, {2, 6.78f}, {3, 9.0f}};
		YSL(INFO) << "PI";
		YSL(INFO) << YAML::Flow << std::vector<int>{3, 1, 4, 1, 5, 9, 2, 6};
		YSL(INFO) << YAML::EndMap << YAML::EndDoc;
	}

	// comment, literal and implicit/explicit new line
	{
		YSL(INFO) << YAML::ThreadFrame("Hello Newline") << YAML::BeginMap;
		YSL(INFO) << YAML::Comment("This is a comment");
		YSL(INFO) << "glog newline" << -1;
		LOG(INFO) << std::endl;
		YSL(INFO) << "ysl newline" << true;
		YSL(INFO) << YAML::Newline;
		YSL(INFO) << "newline and literal" << YAML::Literal << "multi\nline\nliteral";
		YSL(INFO) << YAML::EndMap << YAML::Newline;
	}

	YSL_AT_LEVEL(2) << YAML::ThreadFrame("At level 2") << YAML::EndDoc;

	// logging level and LOG_IF like
	for (int loop = 0; loop < 10; ++loop)
	{
		YSL_IF(ERROR, loop & 1) << YAML::ThreadFrame("Loop " + std::to_string(loop));
		YSL_IF(ERROR, loop & 1) << YAML::BeginSeq;
		YSL_IF(WARNING, loop & 1) << YAML::Flow << std::vector<int>(loop, loop);
		YSL_IF(ERROR, loop & 1) << YAML::EndSeq;
		VYSL_IF(1, loop & 3) << std::vector<int>(loop, loop);
	}

	// threaded logging
	const int  n      = 4;
	const int  m      = 1000;
	const auto worker = [](int idx) {
		YAML::StreamLogger::set_thread_format(YAML::DoubleQuoted); // better performance
		YAML::StreamLogger::set_thread_format(YAML::LoggerFormat::FloatPrecision, 3);

		// interval logging
		for (int loop = 0; loop < m; ++loop)
		{
			YSL(INFO) << YAML::ThreadFrame("Thread " + std::to_string(idx)) << YAML::Flow
					  << YAML::BeginMap;
			auto phase = idx * .1f + loop * .2f;
			YSL(INFO) << "cos" << std::cos(phase) << "sin" << std::sin(phase);
			YSL(INFO) << "log" << std::log(phase) << "exp" << std::exp(phase);
			YSL(INFO) << YAML::EndMap;

			// C++14
			// using namespace std::chrono_literals;
			// std::this_thread::sleep_for(10ms);
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
		}

		YSL(INFO) << YAML::EndDoc;
	};
	std::vector<std::thread> threads;
	threads.reserve(n);
	for (int idx = 0; idx < n; ++idx)
	{
		threads.emplace_back(std::thread{worker, idx});
	}
	for (auto& thread : threads)
	{
		thread.join();
	}

	return 0;
}
