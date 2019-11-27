//#include "../vocaloid/file_reader_node.hpp"
//#include "../vocaloid/file_writer_node.hpp"
//#include "../vocaloid/audio_context.hpp"
//using namespace vocaloid::node;
//int main() {
//	auto ctx = new AudioContext();
//	ctx->SetOutput(OutputType::RECORDER);
//	auto dest = (FileWriterNode*)(ctx->Destination());
//	dest->SetPath("C:\\Users\\Admin\\Desktop\\output.mp3");
//	auto reader = new FileReaderNode(ctx);
//	reader->Open("D:\\projects\\vocaloid\\samples\\example.mp3");
//	reader->Start();
//	ctx->Connect(reader, dest);
//	ctx->Prepare();
//	ctx->Start();
//	getchar();
//	ctx->Close();
//	ctx->Dispose();
//	return 1;
//}