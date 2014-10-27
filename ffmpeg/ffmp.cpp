#include <iostream>
#include <fstream>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
}

using namespace std;

//#define DEBUG 0 //Uncomment in case of debug

#define printERR(x) cerr<<" ------ Error found: "<<x<<endl;

int main(int argc, char *argv[])
{
    try{

        if(argc!=3) throw "Uncorrect number of inputs. (To use this application type: ./ffmp input_file output_file)";


        av_register_all();

        AVCodec *codec;

        //    long index = 0;

        AVFrame* frame = avcodec_alloc_frame();
        if (!frame)
        {
            cout << "Error allocating the frame!";
            return -1;
        }

        AVFormatContext* formatContext = NULL;
        if (av_open_input_file(&formatContext,argv[1],NULL,0,NULL) != 0)
        {
            av_free(frame);
            cout << "Error opening the file!";
            return -1;
        }

        if (av_find_stream_info(formatContext) < 0)
        {
            av_free(frame);
            avformat_close_input(&formatContext);
            cout << "Error finding the stream info!";
            return -1;
        }

        // Find the audio stream (some container files can have multiple streams in them)
        AVStream* audioStream = NULL;
        for (unsigned int i = 0; i < formatContext->nb_streams; ++i)
        {
            if (formatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
            {
                audioStream = formatContext->streams[i];
                break;
            }
        }

        if (audioStream == NULL)
        {
            av_free(frame);
            avformat_close_input(&formatContext);
            cout << "Could not find any audio stream in the file!";
            return -1;
        }

        AVCodecContext* codecContext = audioStream->codec;

        codec = avcodec_find_decoder(codecContext->codec_id);

        if (!codec)
        {
            av_free(frame);
            avformat_close_input(&formatContext);
            cout << "Couldn't find a proper decoder!";
            return -1;
        }
        else if (avcodec_open(codecContext,/*codecContext->*/codec))
        {
            av_free(frame);
            avformat_close_input(&formatContext);
            cout << "Couldn't open the context with the decoder!";
            return -1;
        }

        const char * chString = (codecContext->channels ==1 ? " channel" : " channels");
        cout << "This stream has " << codecContext->channels << chString <<" and a sample rate of " << codecContext->sample_rate << "Hz"<<endl;
        cout << "The data is in the format " << av_get_sample_fmt_name(codecContext->sample_fmt)<<endl;

        AVPacket packet;
        av_init_packet(&packet);

        enum AVSampleFormat src_sample_fmt = codecContext->sample_fmt;

        int64_t src_ch_layout;
        switch(codecContext->channels)
        {
        case 1:
            src_ch_layout = AV_CH_LAYOUT_MONO;
            break;

        case 2:
            src_ch_layout = AV_CH_LAYOUT_STEREO;
            break;

        default:
            cout << "Unknown input channel layout (" << codecContext->channels << " channels)!";
            return -1;
        }


        uint8_t *src_data = NULL;
        int src_nb_channels = 0;
        int src_linesize;
        int src_nb_samples;
        int ret;
        int dst_bufsize;

        ofstream outfile;

        try{
            outfile.exceptions (std::ios::failbit | std::ios::badbit);
            outfile.open(argv[2]);
        }catch(std::ios_base::failure &e){
            printERR("file output not opened");
        }

#ifdef  DEBUG

            int iter = 0;

#endif     /* -----  not in DEBUG  ----- */

        // Read the packets in a loop
        while (av_read_frame(formatContext, &packet) == 0)
        {

            if (packet.stream_index == audioStream->index)
            {

                // Try to decode the packet into a frame
                int frameFinished = 0;
                avcodec_decode_audio4(codecContext, frame, &frameFinished, &packet);

                // Some frames rely on multiple packets, so we have to make sure the frame is finished before we can use it
                if (frameFinished)
                {
                    src_nb_samples = frame->nb_samples;

                    // Allocate source and destination samples buffers
                    src_nb_channels = av_get_channel_layout_nb_channels(src_ch_layout);
                    ret = av_samples_alloc(&src_data,&src_linesize,src_nb_channels,src_nb_samples,codecContext->sample_fmt,1);
                    if (ret < 0)
                    {
                        cout << "Could not allocate source samples!";
                        return -1;
                    }

                    src_data = frame->data[0];

                    dst_bufsize = av_samples_get_buffer_size(&src_linesize, src_nb_channels, src_nb_samples, src_sample_fmt, 1);

                    int sampleSize = av_get_bytes_per_sample (codecContext->sample_fmt);

                    const unsigned char *ptr = reinterpret_cast<const unsigned char *>(src_data);

                    for (int i = 0; i < src_nb_samples; ++i) {
                        for (int j = 0; j < codecContext->channels; ++j) {
                            int32_t value;

                            if (codecContext->sample_fmt == AV_SAMPLE_FMT_U8) {

                                value = *reinterpret_cast<const uint8_t*>(ptr);

                            } else if (codecContext->sample_fmt == AV_SAMPLE_FMT_S16) {

                                value = *(int16_t*)(ptr);

                            } else if (codecContext->sample_fmt == AV_SAMPLE_FMT_S32) {

                                value = *(int32_t*)(ptr);

                            } else if (codecContext->sample_fmt == AV_SAMPLE_FMT_FLT) {

                                value = (*(float*)(ptr) * 0x7fffffff); // assumes 0-1.0

                            } else {
                                cerr<<av_get_sample_fmt_name (codecContext->sample_fmt);
                                throw "Sample format is not supported";
                            }

                            ptr += sampleSize;

#ifdef  DEBUG

            cout<<++iter<<":"<<value<<endl;

#endif     /* -----  not in DEBUG  ----- */

                            outfile<<value<<endl;
                        }
                    }
                }
            }

            // You *must* call av_free_packet() after each call to av_read_frame() or else you'll leak memory
            av_free_packet(&packet);
        }

        // Some codecs will cause frames to be buffered up in the decoding process. If the CODEC_CAP_DELAY flag
        // is set, there can be buffered up frames that need to be flushed, so we'll do that
        if (codecContext->codec->capabilities & CODEC_CAP_DELAY)
        {
            av_init_packet(&packet);
            // Decode all the remaining frames in the buffer, until the end is reached
            int frameFinished = 0;
            while (avcodec_decode_audio4(codecContext, frame, &frameFinished, &packet) >= 0 && frameFinished)
            {
                // TODO ?
                cout << "Remaining frames in the buffer!";
            }
        }

        cout<<"File written successfully, check " << argv[2] << endl;
        outfile.close();

        // Clean up!
        av_free(frame);
        avcodec_close(codecContext);
        avformat_close_input(&formatContext);

    }catch(const char * str){
        printERR(str);
    }

}
