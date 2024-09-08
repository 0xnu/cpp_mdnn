#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>
#include <MiniDNN.h>

#define MAX_FEATURES 14

// Structure to hold the adult dataset
struct Sample {
    Eigen::VectorXd features;
    int label;  // 0 for <=50K, 1 for >50K
    Sample() : features(MAX_FEATURES) {}
};

// Function prototypes
std::vector<Sample> read_csv(const std::string& filename);
void normalize_features(std::vector<Sample>& samples);
void shuffle_samples(std::vector<Sample>& samples);
void prepare_data(const std::vector<Sample>& samples, Eigen::MatrixXd& X, Eigen::MatrixXd& y);

int main() {
    // Read and process the CSV file
    std::vector<Sample> samples = read_csv("data/adult.csv");

    // Normalize the features
    normalize_features(samples);

    // Shuffle the samples
    shuffle_samples(samples);

    // Prepare data for MiniDNN
    Eigen::MatrixXd X, y;
    prepare_data(samples, X, y);

    // Create the neural network
    MiniDNN::Network net;
    net.add_layer(new MiniDNN::FullyConnected<MiniDNN::ReLU>(MAX_FEATURES, 64));
    net.add_layer(new MiniDNN::FullyConnected<MiniDNN::ReLU>(64, 32));
    net.add_layer(new MiniDNN::FullyConnected<MiniDNN::Softmax>(32, 2));  // 2 output classes
    net.set_output(new MiniDNN::BinaryClassEntropy());

    // Set up the optimizer
    MiniDNN::Adam opt;
    opt.m_lrate = 0.001;

    // Set up callback for verbose output
    MiniDNN::VerboseCallback callback;
    net.set_callback(callback);

    // Initialize the network
    net.init(0, 0.01);

    // Train the model
    net.fit(opt, X, y, 32, 100);  // batch size = 32, epochs = 100

    // Evaluate the model
    Eigen::MatrixXd pred = net.predict(X);
    int correct = 0;
    for (int i = 0; i < pred.cols(); ++i) {
        if ((pred(0, i) > pred(1, i)) == (y(0, i) == 1)) {
            correct++;
        }
    }
    std::cout << "Accuracy: " << (double)correct / pred.cols() * 100 << "%" << std::endl;

    return 0;
}

std::vector<Sample> read_csv(const std::string& filename) {
    std::ifstream file(filename);
    std::vector<Sample> samples;
    std::string line;

    // Skip header
    std::getline(file, line);

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        Sample sample;
        int feature_index = 0;

        while (std::getline(iss, token, ',')) {
            if (feature_index < MAX_FEATURES) {
                if (feature_index == 1 || feature_index == 3 || feature_index == 5 ||
                    feature_index == 6 || feature_index == 7 || feature_index == 8 ||
                    feature_index == 9 || feature_index == 13) {
                    // Hash categorical variables
                    sample.features[feature_index] = std::hash<std::string>{}(token) % 1000 / 1000.0;
                } else {
                    sample.features[feature_index] = std::stod(token);
                }
                feature_index++;
            } else {
                // Set label
                sample.label = (token.find(">50K") != std::string::npos) ? 1 : 0;
            }
        }
        samples.push_back(sample);
    }

    return samples;
}

void normalize_features(std::vector<Sample>& samples) {
    Eigen::VectorXd min = samples[0].features, max = samples[0].features;

    for (const auto& sample : samples) {
        min = min.cwiseMin(sample.features);
        max = max.cwiseMax(sample.features);
    }

    for (auto& sample : samples) {
        sample.features = (sample.features - min).cwiseQuotient(max - min);
    }
}

void shuffle_samples(std::vector<Sample>& samples) {
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(samples.begin(), samples.end(), g);
}

void prepare_data(const std::vector<Sample>& samples, Eigen::MatrixXd& X, Eigen::MatrixXd& y) {
    X.resize(MAX_FEATURES, samples.size());
    y.resize(2, samples.size());  // 2 classes

    for (size_t i = 0; i < samples.size(); ++i) {
        X.col(i) = samples[i].features;
        y.col(i) = Eigen::Vector2d(1 - samples[i].label, samples[i].label);  // One-hot encoding
    }
}
