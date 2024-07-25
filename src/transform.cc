#include <edge-impulse-sdk/classifier/ei_run_classifier.h>
#include <nlohmann/json.hpp>
#include <redpanda/transform_sdk.h>

namespace {

std::expected<std::vector<float>, std::error_code>
parse_data_points(redpanda::bytes_view raw) {
  auto parsed = nlohmann::json::parse(std::string_view{raw}, /*cb=*/nullptr,
                                      /*allow_exceptions=*/false);

  if (parsed.is_discarded() || !parsed.is_array()) [[unlikely]] {
    std::println(stderr, "Invalid JSON, discarded={}, is_array={}",
                 parsed.is_discarded(), parsed.is_array());
    return std::unexpected(std::make_error_code(std::errc::bad_message));
  }

  std::vector<float> data_points;
  data_points.reserve(parsed.size());
  for (auto &element : parsed) {
    if (!element.is_number()) [[unlikely]] {
      std::println(stderr, "expected number element in array, got: {}",
                   element.type_name());
      return std::unexpected(std::make_error_code(std::errc::bad_message));
    }
    data_points.push_back(element.get<float>());
  }

  return data_points;
}

std::expected<nlohmann::json, std::error_code>
run_edge_impulse_model(std::vector<float> data) {
  constexpr auto input_frame_size =
      static_cast<size_t>(EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
  if (data.size() != input_frame_size) {
    std::println(stderr, "unexpected number of input elements: got={}, want={}",
                 data.size(), input_frame_size);
    return std::unexpected(std::make_error_code(std::errc::invalid_argument));
  }
  signal_t input{
      .get_data = [data = std::move(data)](size_t offset, size_t length,
                                           float *out_ptr) -> int {
        if (offset + length > data.size()) {
          return EIDSP_OUT_OF_BOUNDS;
        }
        std::copy_n(&data[offset], length, out_ptr);
        return EIDSP_OK;
      },
      .total_length = input_frame_size,
  };
  ei_impulse_result_t result;
  auto error = run_classifier(&input, &result, /*debug=*/false);
  if (error != EI_IMPULSE_OK) {
    std::println(stderr, "error running classifier: {}",
                 std::to_underlying(error));
    return std::unexpected(std::make_error_code(std::errc::operation_canceled));
  }
  nlohmann::json output = nlohmann::json::object();
  for (size_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; ++i) {
    // NOLINTBEGIN
    std::string_view key = ei_classifier_inferencing_categories[i];
    float value = result.classification[i].value;
    // NOLINTEND
    output[key] = value;
  }
  return output;
}

std::error_code edge_impulse_transform(const redpanda::write_event &event,
                                       redpanda::record_writer *writer) {
  auto value = event.record.value.value_or(redpanda::bytes_view());
  auto result = parse_data_points(value).and_then(run_edge_impulse_model);
  if (!result.has_value()) {
    return result.error();
  }
  return writer->write({
      .key = event.record.key,
      .value = std::make_optional<redpanda::bytes_view>(result->dump()),
      .headers = event.record.headers,
  });
}

} // namespace

int main() { redpanda::on_record_written(edge_impulse_transform); }
