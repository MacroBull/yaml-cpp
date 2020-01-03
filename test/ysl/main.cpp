
#include <cmath>
#include <map>
#include <thread>
#include <vector>

#include <yaml-cpp/contrib/ysl.hpp>
#include <yaml-cpp/stlemitter.h>

int main(int /*argc*/, char* argv[])
{
	// setup glog
	FLAGS_logtostderr      = true;
	FLAGS_colorlogtostderr = true;
	FLAGS_minloglevel      = 0;
	FLAGS_v                = 2;
	// FLAGS_log_prefix       = false;

	google::InitGoogleLogging(argv[0]);

	// setup YSL for current thread: use 4 sapces as indent
	YAML::StreamLogger::set_thread_format(YAML::LoggerFormat::Indent, 4);

	// plain glog LOG
	LOG(INFO) << "This is a plain glog record";

	// simple key-value scope
	// YSL(INFO) << key << value;
	{
		YSL_FSCOPE(INFO, "A Simple YSL Frame");
		YSL(INFO) << "name"          // key
				  << "Mark McGwire"; // value
		YSL(INFO) << "hr" << 65;
		YSL(INFO) << "avg" << 0.278f;
	}

	// stl and flow emission, explicit end-of-document
	{
		// manually start a frame
		YSL(INFO) << YAML::ThreadFrame{"Some STL Containers"};
		// setup local precision
		YSL(INFO) << YAML::FloatPrecision(3);
		// start a mapping
		YSL(INFO) << YAML::BeginMap;
		YSL(INFO) << "hello" << std::map<int, float>{{1, 3.4f}, {2, 6.78f}, {3, 9.0f}};
		YSL(INFO) << "PI";
		YSL(INFO) << YAML::Flow << std::vector<int>{3, 1, 4, 1, 5, 9, 2, 6};
		YSL(INFO) << "empty tuple" << std::tuple<>();
		// end the mapping and the frame(explicitly)
		YSL(INFO) << YAML::EndMap << YAML::EndDoc;
	}

	// comment, literal and implicit/explicit new line
	{
		YSL_FSCOPE(INFO, "About Comment, Literal And Newline");
		YSL(INFO) << YAML::Comment("This is a comment");
		LOGC(INFO) << "This is a comment by LOGC";
		YSL(INFO) << "glog newline" << nullptr;
		LOG(INFO) << std::endl;
		YSL(INFO) << "ysl newline" << true;
		YSL(INFO) << YAML::Newline;
		YSL(INFO) << "newline and literal" << YAML::Literal << "multi\nline\nliteral";
	}

	// LOG_AT_LEVEL like
	YSL_AT_LEVEL(2)
			<< YAML::ThreadFrame{"A Veryveryveryveryveryveryveryvery Long Frame At level 2"}
			<< YAML::EndDoc;

	// logging level and LOG_IF like
	for (int loop = 0; loop < 10; ++loop)
	{
		YSL_IF(ERROR, loop & 1) << YAML::ThreadFrame{"Loop " + std::to_string(loop)};
		YSL_IF(ERROR, loop & 1) << YAML::BeginSeq;
		YSL_IF(WARNING, loop & 1) << YAML::Flow << std::vector<int>(loop, loop);
		YSL_IF(ERROR, loop & 1) << YAML::EndSeq;
		VYSL_IF(1, loop & 3) << std::vector<int>(loop, loop);
	}

	// threaded logging
	const int  n      = 4;
	const int  m      = 1000;
	const auto worker = [](int idx) {
		// use quoted string style for better performance
		YAML::StreamLogger::set_thread_format(YAML::DoubleQuoted);
		YAML::StreamLogger::set_thread_format(YAML::LoggerFormat::FloatPrecision, 3);

		// interval logging
		for (int loop = 0; loop < m; ++loop)
		{
			YSL(INFO) << YAML::ThreadFrame{"Thread " + std::to_string(idx)} << YAML::Flow
					  << YAML::BeginMap;
			VYSL_LIC_IF(2, "mod10 counter", loop % 10 == 0);

			auto phase = static_cast<float>(idx) * .1f + static_cast<float>(loop) * .2f;
			YSL(INFO) << "cos" << std::cos(phase) << "sin" << std::sin(phase);
			YSL(INFO) << "log" << std::log(phase) << "exp" << std::exp(phase);
			YSL(INFO) << "complex" << std::complex<int>{idx, loop};
			YSL(INFO) << YAML::EndMap;

			// C++14
			// using namespace std::chrono_literals;
			// std::this_thread::sleep_for(10ms);
			std::this_thread::sleep_for(std::chrono::milliseconds{10});
		}

		YSL(INFO) << YAML::EndDoc; // explicit
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

	LOG(INFO); // HINT: extra flush
	return 0;
}
