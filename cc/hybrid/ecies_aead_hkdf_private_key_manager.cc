// Copyright 2017 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
///////////////////////////////////////////////////////////////////////////////

#include "tink/hybrid/ecies_aead_hkdf_private_key_manager.h"

#include "absl/strings/string_view.h"
#include "tink/hybrid_decrypt.h"
#include "tink/key_manager.h"
#include "tink/hybrid/ecies_aead_hkdf_hybrid_decrypt.h"
#include "tink/util/errors.h"
#include "tink/util/protobuf_helper.h"
#include "tink/util/status.h"
#include "tink/util/statusor.h"
#include "tink/util/validation.h"
#include "proto/ecies_aead_hkdf.pb.h"
#include "proto/tink.pb.h"

namespace crypto {
namespace tink {

using google::crypto::tink::EciesAeadHkdfPrivateKey;
using google::crypto::tink::EciesAeadHkdfKeyFormat;
using google::crypto::tink::EciesAeadHkdfParams;
using google::crypto::tink::KeyData;
using google::crypto::tink::KeyTemplate;
using portable_proto::MessageLite;
using crypto::tink::util::Status;
using crypto::tink::util::StatusOr;

class EciesAeadHkdfPrivateKeyFactory : public KeyFactory {
 public:
  EciesAeadHkdfPrivateKeyFactory() {}

  // Generates a new random EciesAeadHkdfPrivateKey, based on
  // the given 'key_format', which must contain EciesAeadHkdfKeyFormat-proto.
  crypto::tink::util::StatusOr<std::unique_ptr<portable_proto::MessageLite>>
  NewKey(const portable_proto::MessageLite& key_format) const override;

  // Generates a new random EciesAeadHkdfPrivateKey, based on
  // the given 'serialized_key_format', which must contain
  // EciesAeadHkdfKeyFormat-proto.
  crypto::tink::util::StatusOr<std::unique_ptr<portable_proto::MessageLite>>
  NewKey(absl::string_view serialized_key_format) const override;

  // Generates a new random EciesAeadHkdfPrivateKey based on
  // the given 'serialized_key_format' (which must contain
  // EciesAeadHkdfKeyFormat-proto), and wraps it in a KeyData-proto.
  crypto::tink::util::StatusOr<std::unique_ptr<google::crypto::tink::KeyData>>
  NewKeyData(absl::string_view serialized_key_format) const override;
};

StatusOr<std::unique_ptr<MessageLite>> EciesAeadHkdfPrivateKeyFactory::NewKey(
    const portable_proto::MessageLite& key_format) const {
  return util::Status(util::error::UNIMPLEMENTED, "not implemented yet");
}

StatusOr<std::unique_ptr<MessageLite>> EciesAeadHkdfPrivateKeyFactory::NewKey(
    absl::string_view serialized_key_format) const {
  return util::Status(util::error::UNIMPLEMENTED, "not implemented yet");
}

StatusOr<std::unique_ptr<KeyData>> EciesAeadHkdfPrivateKeyFactory::NewKeyData(
    absl::string_view serialized_key_format) const {
  return util::Status(util::error::UNIMPLEMENTED, "not implemented yet");
}

constexpr char EciesAeadHkdfPrivateKeyManager::kKeyTypePrefix[];
constexpr char EciesAeadHkdfPrivateKeyManager::kKeyType[];
constexpr uint32_t EciesAeadHkdfPrivateKeyManager::kVersion;

EciesAeadHkdfPrivateKeyManager::EciesAeadHkdfPrivateKeyManager()
    : key_type_(kKeyType), key_factory_(new EciesAeadHkdfPrivateKeyFactory()) {
}

const std::string& EciesAeadHkdfPrivateKeyManager::get_key_type() const {
  return key_type_;
}

const KeyFactory& EciesAeadHkdfPrivateKeyManager::get_key_factory() const {
  return *key_factory_;
}

uint32_t EciesAeadHkdfPrivateKeyManager::get_version() const {
  return kVersion;
}

StatusOr<std::unique_ptr<HybridDecrypt>>
EciesAeadHkdfPrivateKeyManager::GetPrimitive(const KeyData& key_data) const {
  if (DoesSupport(key_data.type_url())) {
    EciesAeadHkdfPrivateKey ecies_private_key;
    if (!ecies_private_key.ParseFromString(key_data.value())) {
      return ToStatusF(util::error::INVALID_ARGUMENT,
                       "Could not parse key_data.value as key type '%s'.",
                       key_data.type_url().c_str());
    }
    return GetPrimitiveImpl(ecies_private_key);
  } else {
    return ToStatusF(util::error::INVALID_ARGUMENT,
                     "Key type '%s' is not supported by this manager.",
                     key_data.type_url().c_str());
  }
}

StatusOr<std::unique_ptr<HybridDecrypt>>
EciesAeadHkdfPrivateKeyManager::GetPrimitive(const MessageLite& key) const {
  std::string key_type = std::string(kKeyTypePrefix) + key.GetTypeName();
  if (DoesSupport(key_type)) {
    const EciesAeadHkdfPrivateKey& ecies_private_key =
        reinterpret_cast<const EciesAeadHkdfPrivateKey&>(key);
    return GetPrimitiveImpl(ecies_private_key);
  } else {
    return ToStatusF(util::error::INVALID_ARGUMENT,
                     "Key type '%s' is not supported by this manager.",
                     key_type.c_str());
  }
}

StatusOr<std::unique_ptr<HybridDecrypt>>
EciesAeadHkdfPrivateKeyManager::GetPrimitiveImpl(
    const EciesAeadHkdfPrivateKey& ecies_private_key) const {
  Status status = Validate(ecies_private_key);
  if (!status.ok()) return status;
  auto ecies_result = EciesAeadHkdfHybridDecrypt::New(ecies_private_key);
  if (!ecies_result.ok()) return ecies_result.status();
  return std::move(ecies_result.ValueOrDie());
}

// static
Status EciesAeadHkdfPrivateKeyManager::Validate(
    const EciesAeadHkdfParams& params) {
  return Status::OK;
}

// static
Status EciesAeadHkdfPrivateKeyManager::Validate(
    const EciesAeadHkdfPrivateKey& key) {
  Status status = ValidateVersion(key.version(), kVersion);
  if (!status.ok()) return status;
  return Validate(key.public_key().params());
}

// static
Status EciesAeadHkdfPrivateKeyManager::Validate(
    const EciesAeadHkdfKeyFormat& key_format) {
  return Status::OK;
}

}  // namespace tink
}  // namespace crypto
