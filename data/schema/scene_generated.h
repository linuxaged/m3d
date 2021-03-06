// automatically generated by the FlatBuffers compiler, do not modify

#ifndef FLATBUFFERS_GENERATED_SCENE_M3D_SCHEMA_H_
#define FLATBUFFERS_GENERATED_SCENE_M3D_SCHEMA_H_

#include "flatbuffers/flatbuffers.h"

namespace m3d {
namespace schema {

struct SVector3;

struct STransform;

struct SScene;

struct SModel;

MANUALLY_ALIGNED_STRUCT(4) SVector3 FLATBUFFERS_FINAL_CLASS {
 private:
  float x_;
  float y_;
  float z_;

 public:
  SVector3() { memset(this, 0, sizeof(SVector3)); }
  SVector3(const SVector3 &_o) { memcpy(this, &_o, sizeof(SVector3)); }
  SVector3(float _x, float _y, float _z)
    : x_(flatbuffers::EndianScalar(_x)), y_(flatbuffers::EndianScalar(_y)), z_(flatbuffers::EndianScalar(_z)) { }

  float x() const { return flatbuffers::EndianScalar(x_); }
  float y() const { return flatbuffers::EndianScalar(y_); }
  float z() const { return flatbuffers::EndianScalar(z_); }
};
STRUCT_END(SVector3, 12);

MANUALLY_ALIGNED_STRUCT(4) STransform FLATBUFFERS_FINAL_CLASS {
 private:
  SVector3 position_;
  SVector3 rotation_;
  SVector3 scale_;

 public:
  STransform() { memset(this, 0, sizeof(STransform)); }
  STransform(const STransform &_o) { memcpy(this, &_o, sizeof(STransform)); }
  STransform(const SVector3 &_position, const SVector3 &_rotation, const SVector3 &_scale)
    : position_(_position), rotation_(_rotation), scale_(_scale) { }

  const SVector3 &position() const { return position_; }
  const SVector3 &rotation() const { return rotation_; }
  const SVector3 &scale() const { return scale_; }
};
STRUCT_END(STransform, 36);

struct SScene FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_MODELS = 4
  };
  const flatbuffers::Vector<flatbuffers::Offset<SModel>> *models() const { return GetPointer<const flatbuffers::Vector<flatbuffers::Offset<SModel>> *>(VT_MODELS); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_MODELS) &&
           verifier.Verify(models()) &&
           verifier.VerifyVectorOfTables(models()) &&
           verifier.EndTable();
  }
};

struct SSceneBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_models(flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<SModel>>> models) { fbb_.AddOffset(SScene::VT_MODELS, models); }
  SSceneBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  SSceneBuilder &operator=(const SSceneBuilder &);
  flatbuffers::Offset<SScene> Finish() {
    auto o = flatbuffers::Offset<SScene>(fbb_.EndTable(start_, 1));
    return o;
  }
};

inline flatbuffers::Offset<SScene> CreateSScene(flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::Vector<flatbuffers::Offset<SModel>>> models = 0) {
  SSceneBuilder builder_(_fbb);
  builder_.add_models(models);
  return builder_.Finish();
}

inline flatbuffers::Offset<SScene> CreateSSceneDirect(flatbuffers::FlatBufferBuilder &_fbb,
    const std::vector<flatbuffers::Offset<SModel>> *models = nullptr) {
  return CreateSScene(_fbb, models ? _fbb.CreateVector<flatbuffers::Offset<SModel>>(*models) : 0);
}

struct SModel FLATBUFFERS_FINAL_CLASS : private flatbuffers::Table {
  enum {
    VT_NAME = 4,
    VT_TRANSFORM = 6
  };
  const flatbuffers::String *name() const { return GetPointer<const flatbuffers::String *>(VT_NAME); }
  const STransform *transform() const { return GetStruct<const STransform *>(VT_TRANSFORM); }
  bool Verify(flatbuffers::Verifier &verifier) const {
    return VerifyTableStart(verifier) &&
           VerifyField<flatbuffers::uoffset_t>(verifier, VT_NAME) &&
           verifier.Verify(name()) &&
           VerifyField<STransform>(verifier, VT_TRANSFORM) &&
           verifier.EndTable();
  }
};

struct SModelBuilder {
  flatbuffers::FlatBufferBuilder &fbb_;
  flatbuffers::uoffset_t start_;
  void add_name(flatbuffers::Offset<flatbuffers::String> name) { fbb_.AddOffset(SModel::VT_NAME, name); }
  void add_transform(const STransform *transform) { fbb_.AddStruct(SModel::VT_TRANSFORM, transform); }
  SModelBuilder(flatbuffers::FlatBufferBuilder &_fbb) : fbb_(_fbb) { start_ = fbb_.StartTable(); }
  SModelBuilder &operator=(const SModelBuilder &);
  flatbuffers::Offset<SModel> Finish() {
    auto o = flatbuffers::Offset<SModel>(fbb_.EndTable(start_, 2));
    return o;
  }
};

inline flatbuffers::Offset<SModel> CreateSModel(flatbuffers::FlatBufferBuilder &_fbb,
    flatbuffers::Offset<flatbuffers::String> name = 0,
    const STransform *transform = 0) {
  SModelBuilder builder_(_fbb);
  builder_.add_transform(transform);
  builder_.add_name(name);
  return builder_.Finish();
}

inline flatbuffers::Offset<SModel> CreateSModelDirect(flatbuffers::FlatBufferBuilder &_fbb,
    const char *name = nullptr,
    const STransform *transform = 0) {
  return CreateSModel(_fbb, name ? _fbb.CreateString(name) : 0, transform);
}

inline const m3d::schema::SScene *GetSScene(const void *buf) {
  return flatbuffers::GetRoot<m3d::schema::SScene>(buf);
}

inline bool VerifySSceneBuffer(flatbuffers::Verifier &verifier) {
  return verifier.VerifyBuffer<m3d::schema::SScene>(nullptr);
}

inline void FinishSSceneBuffer(flatbuffers::FlatBufferBuilder &fbb, flatbuffers::Offset<m3d::schema::SScene> root) {
  fbb.Finish(root);
}

}  // namespace schema
}  // namespace m3d

#endif  // FLATBUFFERS_GENERATED_SCENE_M3D_SCHEMA_H_
