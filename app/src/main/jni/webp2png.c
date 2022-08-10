#include <stdlib.h>
#include <jni.h>
#include <android/log.h>
#include <webp/decode.h>
#include <webp/demux.h>
#include <webp/mux.h>
#include <assert.h>
#include "example_util.h"
#include "swig_utils.h"
#include <stdio.h>
#include <string.h>
#include <libwebp/imageio/imageio_util.h>


static const char *const kErrorMessages[-WEBP_MUX_NOT_ENOUGH_DATA + 1] = {
        "WEBP_MUX_NOT_FOUND", "WEBP_MUX_INVALID_ARGUMENT", "WEBP_MUX_BAD_DATA",
        "WEBP_MUX_MEMORY_ERROR", "WEBP_MUX_NOT_ENOUGH_DATA"
};

static const char *ErrorString(WebPMuxError err) {
    assert(err <= WEBP_MUX_NOT_FOUND && err >= WEBP_MUX_NOT_ENOUGH_DATA);
    return kErrorMessages[-err];
}

static int CreateMux(const char *const filename, WebPMux **mux) {
    WebPData bitstream;
    assert(mux != NULL);
    if (!ExUtilReadFileToWebPData(filename, &bitstream)) return 0;
    *mux = WebPMuxCreate(&bitstream, 1);
    WebPDataClear(&bitstream);
    if (*mux != NULL) return 1;
    fprintf(stderr, "Failed to create mux object from file %s.\n",
            (const char *) filename);
    return 0;
}

static int WriteData(const char *filename, const WebPData *const webpdata) {
    int ok = 0;
    FILE *fout = strcmp(filename, "-") ? fopen(filename, "wb")
                                       : ImgIoUtilSetBinaryMode(stdout);
    if (fout == NULL) {
        fprintf(stderr, "Error opening output WebP file %s!\n",
                (const char *) filename);
        return 0;
    }
    if (fwrite(webpdata->bytes, webpdata->size, 1, fout) != 1) {
        fprintf(stderr, "Error writing file %s!\n", (const char *) filename);
    } else {
        fprintf(stderr, "Saved file %s (%d bytes)\n",
                (const char *) filename, (int) webpdata->size);
        ok = 1;
    }
    if (fout != stdout) fclose(fout);
    return ok;
}

static int WriteWebP(WebPMux *const mux, const char *filename) {
    int ok;
    WebPData webp_data;
    const WebPMuxError err = WebPMuxAssemble(mux, &webp_data);
    if (err != WEBP_MUX_OK) {
        fprintf(stderr, "Error (%s) assembling the WebP file.\n", ErrorString(err));
        return 0;
    }
    ok = WriteData(filename, &webp_data);
    WebPDataClear(&webp_data);
    return ok;
}


static int GetFrame(const WebPMux *mux, int frame_num, const char *filename) {
    WebPMuxError err = WEBP_MUX_OK;
    WebPMux *mux_single = NULL;
    int ok = 1;
    int parse_error = 0;
    const WebPChunkId id = WEBP_CHUNK_ANMF;
    WebPMuxFrameInfo info;
    WebPDataInit(&info.bitstream);

    err = WebPMuxGetFrame(mux, frame_num, &info);
    if (err == WEBP_MUX_OK && info.id != id) err = WEBP_MUX_NOT_FOUND;
    if (err != WEBP_MUX_OK) {
        fprintf(stderr, "ERROR (%s): Could not get frame %d.\n", ErrorString(err), frame_num);
        goto ErrGet;
    }

    mux_single = WebPMuxNew();
    if (mux_single == NULL) {
        err = WEBP_MUX_MEMORY_ERROR;
        fprintf(stderr, "ERROR (%s): Could not allocate a mux object.\n", ErrorString(err));
        goto ErrGet;

    }
    err = WebPMuxSetImage(mux_single, &info.bitstream, 1);
    if (err != WEBP_MUX_OK) {
        fprintf(stderr, "ERROR (%s): Could not create single image mux object.\n",
                ErrorString(err));
        goto ErrGet;
    }

    ok = WriteWebP(mux_single, filename);

    ErrGet:
    WebPDataClear(&info.bitstream);
    WebPMuxDelete(mux_single);
    return ok && !parse_error;
}

char *concat(const char *s1, const char *s2) {
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}


jbyteArray
Java_com_example_webp_WebPDecoder_00024Companion_decodeWebpFile(JNIEnv *jenv, jclass jcls,
                                                                jstring file) {

    jbyteArray *jresult = 0;

    WebPData webPData = {0, 0};
    const char *filename = (*jenv)->GetStringUTFChars(jenv, file, 0);
    ExUtilReadFileToWebPData(filename, &webPData);

    WebPAnimDecoderOptions dec_options;
    WebPAnimDecoderOptionsInit(&dec_options);
    // Tune 'dec_options' as needed.
    WebPAnimDecoder *dec = WebPAnimDecoderNew(&webPData, &dec_options);

    const WebPDemuxer *demux = WebPAnimDecoderGetDemuxer(dec);

    // ... (Iterate over all frames).
    WebPIterator iter;

    WebPMux *webPMux;
    CreateMux(filename, &webPMux);

    char* prefix = "/data/user/0/com.example.webp/files/";
    char* suffix = ".png";
    if (WebPDemuxGetFrame(demux, 1, &iter)) {
        do {
            jresult = SWIG_JavaArrayOutSchar(jenv, (uint8_t *) iter.fragment.bytes,
                                             (jsize) iter.fragment.size);
            size_t size = strlen(prefix)+ strlen(suffix) + 32;
            char frame[size];
            snprintf(frame, size, "%s%d%s", prefix, iter.frame_num, suffix);
            GetFrame(webPMux, iter.frame_num, frame);
        } while (WebPDemuxNextFrame(&iter));
        WebPDemuxReleaseIterator(&iter);
    }

    WebPAnimDecoderDelete(dec);

    return jresult;
}





