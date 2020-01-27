#include <string>
#include <cstring>
#include <vector>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <getopt.h>
#include <unistd.h>
#include <libhackrf/hackrf.h>

using namespace std;

const int SAMPLE_RATE = 2000000;
const int SYMBOL_RATE = 2000;

static hackrf_device *device = NULL;
static volatile bool do_exit = false;

vector<int> data;
vector<uint8_t> out_cu8;
vector<int8_t> out_cs8;

void usage(const char *cmd)
{
    fprintf(stderr, "Usage: %s [-c code] [-o file] [-x gain]\n", cmd);
    exit(1);
}

static int tx_callback(hackrf_transfer *transfer) {
    static size_t ind = 0;
    size_t len = transfer->valid_length;
    if (out_cs8.size() - ind < len) {
        len = out_cs8.size() - ind;
    }
    memcpy(transfer->buffer, out_cs8.data() + ind, len);
    ind += len;
    return ind < out_cs8.size() ? 0 : -1;
}

void generate_data(const string &code_str)
{
    vector<int> code;
    for (char const &c: code_str) {
        if (c == '1') {
            code.push_back(1);
        } else if (c == '0') {
            code.push_back(0);
        } else {
            fprintf(stderr, "code must be only zeros and ones\n");
            exit(1);
        }
    }

    for (int i = 0 ; i < 30 ; i++) {
        data.insert(data.end(), code.begin(), code.end());
        data.insert(data.end(), 31, 0);
    }
}

void generate_samples()
{
    int spb = SAMPLE_RATE / SYMBOL_RATE; // samples per bit
    for (int i = 0 ; i < (int) data.size() ; i++) {
        for (int j = 0 ; j < spb ; j++) {
            out_cu8.push_back(data[i] ? 255 : 127);
            out_cu8.push_back(127);
            out_cs8.push_back(data[i] ? 127 : 0);
            out_cs8.push_back(0);
        }
    }
}

static void start_tx(int freq, int tx_gain)
{
    int result = hackrf_init();
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_init() failed: (%d)\n", result);
        exit(1);
    }
    result = hackrf_open(&device);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_open() failed: (%d)\n", result);
        exit(1);
    }
    result = hackrf_set_sample_rate_manual(device, SAMPLE_RATE, 1);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_sample_rate_set() failed: (%d)\n", result);
        exit(1);
    }
    uint32_t baseband_filter_bw_hz = hackrf_compute_baseband_filter_bw_round_down_lt(SAMPLE_RATE);
    result = hackrf_set_baseband_filter_bandwidth(device, baseband_filter_bw_hz);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_baseband_filter_bandwidth_set() failed: (%d)\n", result);
        exit(1);
    }
    result = hackrf_set_freq(device, freq);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_set_freq() failed: (%d)\n", result);
        exit(1);
    }
    // result = hackrf_set_amp_enable(device, 1);
    // if (result != HACKRF_SUCCESS) {
    //     fprintf(stderr, "hackrf_set_amp_enable() failed: (%d)\n", result);
    // }
    result = hackrf_set_txvga_gain(device, tx_gain);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_set_txvga_gain() failed: (%d)\n", result);
        exit(1);
    }
    result = hackrf_start_tx(device, tx_callback, NULL);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_start_tx() failed: (%d)\n", result);
        exit(1);
    }
}

static void stop_tx()
{
    int result = hackrf_stop_tx(device);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_stop_tx() failed: (%d)\n", result);
    }
    result = hackrf_close(device);
    if (result != HACKRF_SUCCESS) {
        fprintf(stderr, "hackrf_close() failed: (%d)\n", result);
    }
}

template<typename T>
void save_to_file(const string &fname, vector<T> &out)
{
    printf("Saving to %s\n", fname.c_str());
    FILE *f = fopen(fname.c_str(), "wb");
    fwrite(out.data(), 1, out.size(), f);
    fclose(f);
}

int main(int argc, char *argv[])
{
    int opt;
    int freq = 433920000;
    int tx_gain = 20;
    string fname;
    string code;

    while ((opt = getopt(argc, argv, "c:o:x:")) != -1) {
        switch (opt) {
            case 'c':
                code = optarg;
                break;
            case 'o':
                fname = optarg;
                break;
            case 'x':
                tx_gain = atoi(optarg);
                break;
            default:
                usage(argv[0]);
                return 1;
        }
    }
    if (code.empty()) {
        fprintf(stderr, "Missing code\n");
        usage(argv[0]);
        return 1;
    }

    generate_data(code);
    generate_samples();

    if (!fname.empty()) {
        save_to_file(fname + ".cu8", out_cu8);
        save_to_file(fname + ".cs8", out_cs8);
        return 0;
    }

    start_tx(freq, tx_gain);
    while ((hackrf_is_streaming(device) == HACKRF_TRUE) && !do_exit) {
        usleep(10000);
    }
    stop_tx();
    return 0;
}
