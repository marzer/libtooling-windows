//===- LangOptions.h - C Language Family Language Options -------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Defines the clang::LangOptions interface.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_BASIC_LANGOPTIONS_H
#define LLVM_CLANG_BASIC_LANGOPTIONS_H

#include "clang/Basic/CommentOptions.h"
#include "clang/Basic/LLVM.h"
#include "clang/Basic/LangStandard.h"
#include "clang/Basic/ObjCRuntime.h"
#include "clang/Basic/Sanitizers.h"
#include "clang/Basic/TargetCXXABI.h"
#include "clang/Basic/Visibility.h"
#include "llvm/ADT/FloatingPointMode.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/Triple.h"
#include <string>
#include <vector>

namespace clang {

/// Bitfields of LangOptions, split out from LangOptions in order to ensure that
/// this large collection of bitfields is a trivial class type.
class LangOptionsBase {
  friend class CompilerInvocation;

public:
  // Define simple language options (with no accessors).
#define LANGOPT(Name, Bits, Default, Description) unsigned Name : Bits;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description)
#include "clang/Basic/LangOptions.def"

protected:
  // Define language options of enumeration type. These are private, and will
  // have accessors (below).
#define LANGOPT(Name, Bits, Default, Description)
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  unsigned Name : Bits;
#include "clang/Basic/LangOptions.def"
};

/// In the Microsoft ABI, this controls the placement of virtual displacement
/// members used to implement virtual inheritance.
enum class MSVtorDispMode { Never, ForVBaseOverride, ForVFTable };

/// Shader programs run in specific pipeline stages.
/// The order of these values matters, and must be kept in sync with the
/// Triple Environment enum in llvm::Triple. The ordering is enforced in
///  static_asserts in Triple.cpp and in clang/Basic/HLSLRuntime.h.
enum class ShaderStage {
  Pixel = 0,
  Vertex,
  Geometry,
  Hull,
  Domain,
  Compute,
  Library,
  RayGeneration,
  Intersection,
  AnyHit,
  ClosestHit,
  Miss,
  Callable,
  Mesh,
  Amplification,
  Invalid,
};

/// Keeps track of the various options that can be
/// enabled, which controls the dialect of C or C++ that is accepted.
class LangOptions : public LangOptionsBase {
public:
  using Visibility = clang::Visibility;
  using RoundingMode = llvm::RoundingMode;

  enum GCMode { NonGC, GCOnly, HybridGC };
  enum StackProtectorMode { SSPOff, SSPOn, SSPStrong, SSPReq };

  // Automatic variables live on the stack, and when trivial they're usually
  // uninitialized because it's undefined behavior to use them without
  // initializing them.
  enum class TrivialAutoVarInitKind { Uninitialized, Zero, Pattern };

  enum SignedOverflowBehaviorTy {
    // Default C standard behavior.
    SOB_Undefined,

    // -fwrapv
    SOB_Defined,

    // -ftrapv
    SOB_Trapping
  };

  // FIXME: Unify with TUKind.
  enum CompilingModuleKind {
    /// Not compiling a module interface at all.
    CMK_None,

    /// Compiling a module from a module map.
    CMK_ModuleMap,

    /// Compiling a module from a list of header files.
    CMK_HeaderModule,

    /// Compiling a module header unit.
    CMK_HeaderUnit,

    /// Compiling a C++ modules interface unit.
    CMK_ModuleInterface,
  };

  enum PragmaMSPointersToMembersKind {
    PPTMK_BestCase,
    PPTMK_FullGeneralitySingleInheritance,
    PPTMK_FullGeneralityMultipleInheritance,
    PPTMK_FullGeneralityVirtualInheritance
  };

  using MSVtorDispMode = clang::MSVtorDispMode;

  enum DefaultCallingConvention {
    DCC_None,
    DCC_CDecl,
    DCC_FastCall,
    DCC_StdCall,
    DCC_VectorCall,
    DCC_RegCall
  };

  enum AddrSpaceMapMangling { ASMM_Target, ASMM_On, ASMM_Off };

  // Corresponds to _MSC_VER
  enum MSVCMajorVersion {
    MSVC2010 = 1600,
    MSVC2012 = 1700,
    MSVC2013 = 1800,
    MSVC2015 = 1900,
    MSVC2017 = 1910,
    MSVC2017_5 = 1912,
    MSVC2017_7 = 1914,
    MSVC2019 = 1920,
    MSVC2019_5 = 1925,
    MSVC2019_8 = 1928,
  };

  enum SYCLMajorVersion {
    SYCL_None,
    SYCL_2017,
    SYCL_2020,
    // The "default" SYCL version to be used when none is specified on the
    // frontend command line.
    SYCL_Default = SYCL_2020
  };

  enum HLSLLangStd {
    HLSL_Unset = 0,
    HLSL_2015 = 2015,
    HLSL_2016 = 2016,
    HLSL_2017 = 2017,
    HLSL_2018 = 2018,
    HLSL_2021 = 2021,
    HLSL_202x = 2029,
  };

  /// Clang versions with different platform ABI conformance.
  enum class ClangABI {
    /// Attempt to be ABI-compatible with code generated by Clang 3.8.x
    /// (SVN r257626). This causes <1 x long long> to be passed in an
    /// integer register instead of an SSE register on x64_64.
    Ver3_8,

    /// Attempt to be ABI-compatible with code generated by Clang 4.0.x
    /// (SVN r291814). This causes move operations to be ignored when
    /// determining whether a class type can be passed or returned directly.
    Ver4,

    /// Attempt to be ABI-compatible with code generated by Clang 6.0.x
    /// (SVN r321711). This causes determination of whether a type is
    /// standard-layout to ignore collisions between empty base classes
    /// and between base classes and member subobjects, which affects
    /// whether we reuse base class tail padding in some ABIs.
    Ver6,

    /// Attempt to be ABI-compatible with code generated by Clang 7.0.x
    /// (SVN r338536). This causes alignof (C++) and _Alignof (C11) to be
    /// compatible with __alignof (i.e., return the preferred alignment)
    /// rather than returning the required alignment.
    Ver7,

    /// Attempt to be ABI-compatible with code generated by Clang 9.0.x
    /// (SVN r351319). This causes vectors of __int128 to be passed in memory
    /// instead of passing in multiple scalar registers on x86_64 on Linux and
    /// NetBSD.
    Ver9,

    /// Attempt to be ABI-compatible with code generated by Clang 11.0.x
    /// (git 2e10b7a39b93). This causes clang to pass unions with a 256-bit
    /// vector member on the stack instead of using registers, to not properly
    /// mangle substitutions for template names in some cases, and to mangle
    /// declaration template arguments without a cast to the parameter type
    /// even when that can lead to mangling collisions.
    Ver11,

    /// Attempt to be ABI-compatible with code generated by Clang 12.0.x
    /// (git 8e464dd76bef). This causes clang to mangle lambdas within
    /// global-scope inline variables incorrectly.
    Ver12,

    /// Attempt to be ABI-compatible with code generated by Clang 14.0.x.
    /// This causes clang to:
    ///   - mangle dependent nested names incorrectly.
    ///   - make trivial only those defaulted copy constructors with a
    ///     parameter-type-list equivalent to the parameter-type-list of an
    ///     implicit declaration.
    Ver14,

    /// Attempt to be ABI-compatible with code generated by Clang 15.0.x.
    /// This causes clang to:
    ///   - Reverse the implementation for DR692, DR1395 and DR1432.
    ///   - pack non-POD members of packed structs.
    ///   - consider classes with defaulted special member functions non-pod.
    Ver15,

    /// Conform to the underlying platform's C and C++ ABIs as closely
    /// as we can.
    Latest
  };

  enum class CoreFoundationABI {
    /// No interoperability ABI has been specified
    Unspecified,
    /// CoreFoundation does not have any language interoperability
    Standalone,
    /// Interoperability with the ObjectiveC runtime
    ObjectiveC,
    /// Interoperability with the latest known version of the Swift runtime
    Swift,
    /// Interoperability with the Swift 5.0 runtime
    Swift5_0,
    /// Interoperability with the Swift 4.2 runtime
    Swift4_2,
    /// Interoperability with the Swift 4.1 runtime
    Swift4_1,
  };

  enum FPModeKind {
    // Disable the floating point pragma
    FPM_Off,

    // Enable the floating point pragma
    FPM_On,

    // Aggressively fuse FP ops (E.g. FMA) disregarding pragmas.
    FPM_Fast,

    // Aggressively fuse FP ops and honor pragmas.
    FPM_FastHonorPragmas
  };

  /// Possible floating point exception behavior.
  enum FPExceptionModeKind {
    /// Assume that floating-point exceptions are masked.
    FPE_Ignore,
    /// Transformations do not cause new exceptions but may hide some.
    FPE_MayTrap,
    /// Strictly preserve the floating-point exception semantics.
    FPE_Strict,
    /// Used internally to represent initial unspecified value.
    FPE_Default
  };

  /// Possible float expression evaluation method choices.
  enum FPEvalMethodKind {
    /// The evaluation method cannot be determined or is inconsistent for this
    /// target.
    FEM_Indeterminable = -1,
    /// Use the declared type for fp arithmetic.
    FEM_Source = 0,
    /// Use the type double for fp arithmetic.
    FEM_Double = 1,
    /// Use extended type for fp arithmetic.
    FEM_Extended = 2,
    /// Used only for FE option processing; this is only used to indicate that
    /// the user did not specify an explicit evaluation method on the command
    /// line and so the target should be queried for its default evaluation
    /// method instead.
    FEM_UnsetOnCommandLine = 3
  };

  /// Possible exception handling behavior.
  enum class ExceptionHandlingKind { None, SjLj, WinEH, DwarfCFI, Wasm };

  enum class LaxVectorConversionKind {
    /// Permit no implicit vector bitcasts.
    None,
    /// Permit vector bitcasts between integer vectors with different numbers
    /// of elements but the same total bit-width.
    Integer,
    /// Permit vector bitcasts between all vectors with the same total
    /// bit-width.
    All,
  };

  enum class AltivecSrcCompatKind {
    // All vector compares produce scalars except vector pixel and vector bool.
    // The types vector pixel and vector bool return vector results.
    Mixed,
    // All vector compares produce vector results as in GCC.
    GCC,
    // All vector compares produce scalars as in XL.
    XL,
    // Default clang behaviour.
    Default = Mixed,
  };

  enum class SignReturnAddressScopeKind {
    /// No signing for any function.
    None,
    /// Sign the return address of functions that spill LR.
    NonLeaf,
    /// Sign the return address of all functions,
    All
  };

  enum class SignReturnAddressKeyKind {
    /// Return address signing uses APIA key.
    AKey,
    /// Return address signing uses APIB key.
    BKey
  };

  enum class ThreadModelKind {
    /// POSIX Threads.
    POSIX,
    /// Single Threaded Environment.
    Single
  };

  enum class ExtendArgsKind {
    /// Integer arguments are sign or zero extended to 32/64 bits
    /// during default argument promotions.
    ExtendTo32,
    ExtendTo64
  };

  enum class GPUDefaultStreamKind {
    /// Legacy default stream
    Legacy,
    /// Per-thread default stream
    PerThread,
  };

  enum class DefaultVisiblityExportMapping {
    None,
    /// map only explicit default visibilities to exported
    Explicit,
    /// map all default visibilities to exported
    All,
  };

  enum class StrictFlexArraysLevelKind {
    /// Any trailing array member is a FAM.
    Default = 0,
    /// Any trailing array member of undefined, 0, or 1 size is a FAM.
    OneZeroOrIncomplete = 1,
    /// Any trailing array member of undefined or 0 size is a FAM.
    ZeroOrIncomplete = 2,
    /// Any trailing array member of undefined size is a FAM.
    IncompleteOnly = 3,
  };

public:
  /// The used language standard.
  LangStandard::Kind LangStd;

  /// Set of enabled sanitizers.
  SanitizerSet Sanitize;
  /// Is at least one coverage instrumentation type enabled.
  bool SanitizeCoverage = false;

  /// Paths to files specifying which objects
  /// (files, functions, variables) should not be instrumented.
  std::vector<std::string> NoSanitizeFiles;

  /// Paths to the XRay "always instrument" files specifying which
  /// objects (files, functions, variables) should be imbued with the XRay
  /// "always instrument" attribute.
  /// WARNING: This is a deprecated field and will go away in the future.
  std::vector<std::string> XRayAlwaysInstrumentFiles;

  /// Paths to the XRay "never instrument" files specifying which
  /// objects (files, functions, variables) should be imbued with the XRay
  /// "never instrument" attribute.
  /// WARNING: This is a deprecated field and will go away in the future.
  std::vector<std::string> XRayNeverInstrumentFiles;

  /// Paths to the XRay attribute list files, specifying which objects
  /// (files, functions, variables) should be imbued with the appropriate XRay
  /// attribute(s).
  std::vector<std::string> XRayAttrListFiles;

  /// Paths to special case list files specifying which entities
  /// (files, functions) should or should not be instrumented.
  std::vector<std::string> ProfileListFiles;

  clang::ObjCRuntime ObjCRuntime;

  CoreFoundationABI CFRuntime = CoreFoundationABI::Unspecified;

  std::string ObjCConstantStringClass;

  /// The name of the handler function to be called when -ftrapv is
  /// specified.
  ///
  /// If none is specified, abort (GCC-compatible behaviour).
  std::string OverflowHandler;

  /// The module currently being compiled as specified by -fmodule-name.
  std::string ModuleName;

  /// The name of the current module, of which the main source file
  /// is a part. If CompilingModule is set, we are compiling the interface
  /// of this module, otherwise we are compiling an implementation file of
  /// it. This starts as ModuleName in case -fmodule-name is provided and
  /// changes during compilation to reflect the current module.
  std::string CurrentModule;

  /// The names of any features to enable in module 'requires' decls
  /// in addition to the hard-coded list in Module.cpp and the target features.
  ///
  /// This list is sorted.
  std::vector<std::string> ModuleFeatures;

  /// Options for parsing comments.
  CommentOptions CommentOpts;

  /// A list of all -fno-builtin-* function names (e.g., memset).
  std::vector<std::string> NoBuiltinFuncs;

  /// A prefix map for __FILE__, __BASE_FILE__ and __builtin_FILE().
  std::map<std::string, std::string, std::greater<std::string>> MacroPrefixMap;

  /// Triples of the OpenMP targets that the host code codegen should
  /// take into account in order to generate accurate offloading descriptors.
  std::vector<llvm::Triple> OMPTargetTriples;

  /// Name of the IR file that contains the result of the OpenMP target
  /// host code generation.
  std::string OMPHostIRFile;

  /// The user provided compilation unit ID, if non-empty. This is used to
  /// externalize static variables which is needed to support accessing static
  /// device variables in host code for single source offloading languages
  /// like CUDA/HIP.
  std::string CUID;

  /// C++ ABI to compile with, if specified by the frontend through -fc++-abi=.
  /// This overrides the default ABI used by the target.
  llvm::Optional<TargetCXXABI::Kind> CXXABI;

  /// Indicates whether the front-end is explicitly told that the
  /// input is a header file (i.e. -x c-header).
  bool IsHeaderFile = false;

  /// The default stream kind used for HIP kernel launching.
  GPUDefaultStreamKind GPUDefaultStream;

  /// The seed used by the randomize structure layout feature.
  std::string RandstructSeed;

  /// Indicates whether the __FILE__ macro should use the target's
  /// platform-specific file separator or whether it should use the build
  /// environment's platform-specific file separator.
  ///
  /// The plaform-specific path separator is the backslash(\) for Windows and
  /// forward slash (/) elsewhere.
  bool UseTargetPathSeparator = false;

  LangOptions();

  /// Set language defaults for the given input language and
  /// language standard in the given LangOptions object.
  ///
  /// \param Opts - The LangOptions object to set up.
  /// \param Lang - The input language.
  /// \param T - The target triple.
  /// \param Includes - If the language requires extra headers to be implicitly
  ///                   included, they will be appended to this list.
  /// \param LangStd - The input language standard.
  static void
  setLangDefaults(LangOptions &Opts, Language Lang, const llvm::Triple &T,
                  std::vector<std::string> &Includes,
                  LangStandard::Kind LangStd = LangStandard::lang_unspecified);

  // Define accessors/mutators for language options of enumeration type.
#define LANGOPT(Name, Bits, Default, Description)
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  Type get##Name() const { return static_cast<Type>(Name); } \
  void set##Name(Type Value) { Name = static_cast<unsigned>(Value); }
#include "clang/Basic/LangOptions.def"

  /// Are we compiling a module?
  bool isCompilingModule() const {
    return getCompilingModule() != CMK_None;
  }

  /// Are we compiling a standard c++ module interface?
  bool isCompilingModuleInterface() const {
    return getCompilingModule() == CMK_ModuleInterface;
  }

  /// Are we compiling a module implementation?
  bool isCompilingModuleImplementation() const {
    return !isCompilingModule() && !ModuleName.empty();
  }

  /// Do we need to track the owning module for a local declaration?
  bool trackLocalOwningModule() const {
    return isCompilingModule() || ModulesLocalVisibility;
  }

  bool isSignedOverflowDefined() const {
    return getSignedOverflowBehavior() == SOB_Defined;
  }

  bool isSubscriptPointerArithmetic() const {
    return ObjCRuntime.isSubscriptPointerArithmetic() &&
           !ObjCSubscriptingLegacyRuntime;
  }

  bool isCompatibleWithMSVC(MSVCMajorVersion MajorVersion) const {
    return MSCompatibilityVersion >= MajorVersion * 100000U;
  }

  /// Reset all of the options that are not considered when building a
  /// module.
  void resetNonModularOptions();

  /// Is this a libc/libm function that is no longer recognized as a
  /// builtin because a -fno-builtin-* option has been specified?
  bool isNoBuiltinFunc(StringRef Name) const;

  /// True if any ObjC types may have non-trivial lifetime qualifiers.
  bool allowsNonTrivialObjCLifetimeQualifiers() const {
    return ObjCAutoRefCount || ObjCWeak;
  }

  bool assumeFunctionsAreConvergent() const {
    return ConvergentFunctions;
  }

  /// Return the OpenCL C or C++ version as a VersionTuple.
  VersionTuple getOpenCLVersionTuple() const;

  /// Return the OpenCL version that kernel language is compatible with
  unsigned getOpenCLCompatibleVersion() const;

  /// Return the OpenCL C or C++ for OpenCL language name and version
  /// as a string.
  std::string getOpenCLVersionString() const;

  /// Returns true if functions without prototypes or functions with an
  /// identifier list (aka K&R C functions) are not allowed.
  bool requiresStrictPrototypes() const {
    return CPlusPlus || C2x || DisableKNRFunctions;
  }

  /// Returns true if implicit function declarations are allowed in the current
  /// language mode.
  bool implicitFunctionsAllowed() const {
    return !requiresStrictPrototypes() && !OpenCL;
  }

  /// Returns true if implicit int is part of the language requirements.
  bool isImplicitIntRequired() const { return !CPlusPlus && !C99; }

  /// Returns true if implicit int is supported at all.
  bool isImplicitIntAllowed() const { return !CPlusPlus && !C2x; }

  /// Check if return address signing is enabled.
  bool hasSignReturnAddress() const {
    return getSignReturnAddressScope() != SignReturnAddressScopeKind::None;
  }

  /// Check if return address signing uses AKey.
  bool isSignReturnAddressWithAKey() const {
    return getSignReturnAddressKey() == SignReturnAddressKeyKind::AKey;
  }

  /// Check if leaf functions are also signed.
  bool isSignReturnAddressScopeAll() const {
    return getSignReturnAddressScope() == SignReturnAddressScopeKind::All;
  }

  bool hasSjLjExceptions() const {
    return getExceptionHandling() == ExceptionHandlingKind::SjLj;
  }

  bool hasSEHExceptions() const {
    return getExceptionHandling() == ExceptionHandlingKind::WinEH;
  }

  bool hasDWARFExceptions() const {
    return getExceptionHandling() == ExceptionHandlingKind::DwarfCFI;
  }

  bool hasWasmExceptions() const {
    return getExceptionHandling() == ExceptionHandlingKind::Wasm;
  }

  bool isSYCL() const { return SYCLIsDevice || SYCLIsHost; }

  bool hasDefaultVisibilityExportMapping() const {
    return getDefaultVisibilityExportMapping() !=
           DefaultVisiblityExportMapping::None;
  }

  bool isExplicitDefaultVisibilityExportMapping() const {
    return getDefaultVisibilityExportMapping() ==
           DefaultVisiblityExportMapping::Explicit;
  }

  bool isAllDefaultVisibilityExportMapping() const {
    return getDefaultVisibilityExportMapping() ==
           DefaultVisiblityExportMapping::All;
  }

  /// Remap path prefix according to -fmacro-prefix-path option.
  void remapPathPrefix(SmallVectorImpl<char> &Path) const;

  RoundingMode getDefaultRoundingMode() const {
    return RoundingMath ? RoundingMode::Dynamic
                        : RoundingMode::NearestTiesToEven;
  }

  FPExceptionModeKind getDefaultExceptionMode() const {
    FPExceptionModeKind EM = getFPExceptionMode();
    if (EM == FPExceptionModeKind::FPE_Default)
      return FPExceptionModeKind::FPE_Ignore;
    return EM;
  }
};

/// Floating point control options
class FPOptionsOverride;
class FPOptions {
public:
  // We start by defining the layout.
  using storage_type = uint32_t;

  using RoundingMode = llvm::RoundingMode;

  static constexpr unsigned StorageBitSize = 8 * sizeof(storage_type);

  // Define a fake option named "First" so that we have a PREVIOUS even for the
  // real first option.
  static constexpr storage_type FirstShift = 0, FirstWidth = 0;
#define OPTION(NAME, TYPE, WIDTH, PREVIOUS)                                    \
  static constexpr storage_type NAME##Shift =                                  \
      PREVIOUS##Shift + PREVIOUS##Width;                                       \
  static constexpr storage_type NAME##Width = WIDTH;                           \
  static constexpr storage_type NAME##Mask = ((1 << NAME##Width) - 1)          \
                                             << NAME##Shift;
#include "clang/Basic/FPOptions.def"

  static constexpr storage_type TotalWidth = 0
#define OPTION(NAME, TYPE, WIDTH, PREVIOUS) +WIDTH
#include "clang/Basic/FPOptions.def"
      ;
  static_assert(TotalWidth <= StorageBitSize, "Too short type for FPOptions");

private:
  storage_type Value;

  FPOptionsOverride getChangesSlow(const FPOptions &Base) const;

public:
  FPOptions() : Value(0) {
    setFPContractMode(LangOptions::FPM_Off);
    setConstRoundingMode(RoundingMode::Dynamic);
    setSpecifiedExceptionMode(LangOptions::FPE_Default);
  }
  explicit FPOptions(const LangOptions &LO) {
    Value = 0;
    // The language fp contract option FPM_FastHonorPragmas has the same effect
    // as FPM_Fast in frontend. For simplicity, use FPM_Fast uniformly in
    // frontend.
    auto LangOptContractMode = LO.getDefaultFPContractMode();
    if (LangOptContractMode == LangOptions::FPM_FastHonorPragmas)
      LangOptContractMode = LangOptions::FPM_Fast;
    setFPContractMode(LangOptContractMode);
    setRoundingMath(LO.RoundingMath);
    setConstRoundingMode(LangOptions::RoundingMode::Dynamic);
    setSpecifiedExceptionMode(LO.getFPExceptionMode());
    setAllowFPReassociate(LO.AllowFPReassoc);
    setNoHonorNaNs(LO.NoHonorNaNs);
    setNoHonorInfs(LO.NoHonorInfs);
    setNoSignedZero(LO.NoSignedZero);
    setAllowReciprocal(LO.AllowRecip);
    setAllowApproxFunc(LO.ApproxFunc);
    if (getFPContractMode() == LangOptions::FPM_On &&
        getRoundingMode() == llvm::RoundingMode::Dynamic &&
        getExceptionMode() == LangOptions::FPE_Strict)
      // If the FP settings are set to the "strict" model, then
      // FENV access is set to true. (ffp-model=strict)
      setAllowFEnvAccess(true);
    else
      setAllowFEnvAccess(LangOptions::FPM_Off);
  }

  bool allowFPContractWithinStatement() const {
    return getFPContractMode() == LangOptions::FPM_On;
  }
  void setAllowFPContractWithinStatement() {
    setFPContractMode(LangOptions::FPM_On);
  }

  bool allowFPContractAcrossStatement() const {
    return getFPContractMode() == LangOptions::FPM_Fast;
  }
  void setAllowFPContractAcrossStatement() {
    setFPContractMode(LangOptions::FPM_Fast);
  }

  bool isFPConstrained() const {
    return getRoundingMode() != llvm::RoundingMode::NearestTiesToEven ||
           getExceptionMode() != LangOptions::FPE_Ignore ||
           getAllowFEnvAccess();
  }

  RoundingMode getRoundingMode() const {
    RoundingMode RM = getConstRoundingMode();
    if (RM == RoundingMode::Dynamic) {
      // C2x: 7.6.2p3  If the FE_DYNAMIC mode is specified and FENV_ACCESS is
      // "off", the translator may assume that the default rounding mode is in
      // effect.
      if (!getAllowFEnvAccess() && !getRoundingMath())
        RM = RoundingMode::NearestTiesToEven;
    }
    return RM;
  }

  LangOptions::FPExceptionModeKind getExceptionMode() const {
    LangOptions::FPExceptionModeKind EM = getSpecifiedExceptionMode();
    if (EM == LangOptions::FPExceptionModeKind::FPE_Default) {
      if (getAllowFEnvAccess())
        return LangOptions::FPExceptionModeKind::FPE_Strict;
      else
        return LangOptions::FPExceptionModeKind::FPE_Ignore;
    }
    return EM;
  }

  bool operator==(FPOptions other) const { return Value == other.Value; }

  /// Return the default value of FPOptions that's used when trailing
  /// storage isn't required.
  static FPOptions defaultWithoutTrailingStorage(const LangOptions &LO);

  storage_type getAsOpaqueInt() const { return Value; }
  static FPOptions getFromOpaqueInt(storage_type Value) {
    FPOptions Opts;
    Opts.Value = Value;
    return Opts;
  }

  /// Return difference with the given option set.
  FPOptionsOverride getChangesFrom(const FPOptions &Base) const;

  // We can define most of the accessors automatically:
#define OPTION(NAME, TYPE, WIDTH, PREVIOUS)                                    \
  TYPE get##NAME() const {                                                     \
    return static_cast<TYPE>((Value & NAME##Mask) >> NAME##Shift);             \
  }                                                                            \
  void set##NAME(TYPE value) {                                                 \
    Value = (Value & ~NAME##Mask) | (storage_type(value) << NAME##Shift);      \
  }
#include "clang/Basic/FPOptions.def"
  LLVM_DUMP_METHOD void dump();
};

/// Represents difference between two FPOptions values.
///
/// The effect of language constructs changing the set of floating point options
/// is usually a change of some FP properties while leaving others intact. This
/// class describes such changes by keeping information about what FP options
/// are overridden.
///
/// The integral set of FP options, described by the class FPOptions, may be
/// represented as a default FP option set, defined by language standard and
/// command line options, with the overrides introduced by pragmas.
///
/// The is implemented as a value of the new FPOptions plus a mask showing which
/// fields are actually set in it.
class FPOptionsOverride {
  FPOptions Options = FPOptions::getFromOpaqueInt(0);
  FPOptions::storage_type OverrideMask = 0;

public:
  using RoundingMode = llvm::RoundingMode;

  /// The type suitable for storing values of FPOptionsOverride. Must be twice
  /// as wide as bit size of FPOption.
  using storage_type = uint64_t;
  static_assert(sizeof(storage_type) >= 2 * sizeof(FPOptions::storage_type),
                "Too short type for FPOptionsOverride");

  /// Bit mask selecting bits of OverrideMask in serialized representation of
  /// FPOptionsOverride.
  static constexpr storage_type OverrideMaskBits =
      (static_cast<storage_type>(1) << FPOptions::StorageBitSize) - 1;

  FPOptionsOverride() {}
  FPOptionsOverride(const LangOptions &LO)
      : Options(LO), OverrideMask(OverrideMaskBits) {}
  FPOptionsOverride(FPOptions FPO)
      : Options(FPO), OverrideMask(OverrideMaskBits) {}
  FPOptionsOverride(FPOptions FPO, FPOptions::storage_type Mask)
      : Options(FPO), OverrideMask(Mask) {}

  bool requiresTrailingStorage() const { return OverrideMask != 0; }

  void setAllowFPContractWithinStatement() {
    setFPContractModeOverride(LangOptions::FPM_On);
  }

  void setAllowFPContractAcrossStatement() {
    setFPContractModeOverride(LangOptions::FPM_Fast);
  }

  void setDisallowFPContract() {
    setFPContractModeOverride(LangOptions::FPM_Off);
  }

  void setFPPreciseEnabled(bool Value) {
    setAllowFPReassociateOverride(!Value);
    setNoHonorNaNsOverride(!Value);
    setNoHonorInfsOverride(!Value);
    setNoSignedZeroOverride(!Value);
    setAllowReciprocalOverride(!Value);
    setAllowApproxFuncOverride(!Value);
    if (Value)
      /* Precise mode implies fp_contract=on and disables ffast-math */
      setAllowFPContractWithinStatement();
    else
      /* Precise mode disabled sets fp_contract=fast and enables ffast-math */
      setAllowFPContractAcrossStatement();
  }

  storage_type getAsOpaqueInt() const {
    return (static_cast<storage_type>(Options.getAsOpaqueInt())
            << FPOptions::StorageBitSize) |
           OverrideMask;
  }
  static FPOptionsOverride getFromOpaqueInt(storage_type I) {
    FPOptionsOverride Opts;
    Opts.OverrideMask = I & OverrideMaskBits;
    Opts.Options = FPOptions::getFromOpaqueInt(I >> FPOptions::StorageBitSize);
    return Opts;
  }

  FPOptions applyOverrides(FPOptions Base) {
    FPOptions Result =
        FPOptions::getFromOpaqueInt((Base.getAsOpaqueInt() & ~OverrideMask) |
                                     (Options.getAsOpaqueInt() & OverrideMask));
    return Result;
  }

  FPOptions applyOverrides(const LangOptions &LO) {
    return applyOverrides(FPOptions(LO));
  }

  bool operator==(FPOptionsOverride other) const {
    return Options == other.Options && OverrideMask == other.OverrideMask;
  }
  bool operator!=(FPOptionsOverride other) const { return !(*this == other); }

#define OPTION(NAME, TYPE, WIDTH, PREVIOUS)                                    \
  bool has##NAME##Override() const {                                           \
    return OverrideMask & FPOptions::NAME##Mask;                               \
  }                                                                            \
  TYPE get##NAME##Override() const {                                           \
    assert(has##NAME##Override());                                             \
    return Options.get##NAME();                                                \
  }                                                                            \
  void clear##NAME##Override() {                                               \
    /* Clear the actual value so that we don't have spurious differences when  \
     * testing equality. */                                                    \
    Options.set##NAME(TYPE(0));                                                \
    OverrideMask &= ~FPOptions::NAME##Mask;                                    \
  }                                                                            \
  void set##NAME##Override(TYPE value) {                                       \
    Options.set##NAME(value);                                                  \
    OverrideMask |= FPOptions::NAME##Mask;                                     \
  }
#include "clang/Basic/FPOptions.def"
  LLVM_DUMP_METHOD void dump();
};

inline FPOptionsOverride FPOptions::getChangesFrom(const FPOptions &Base) const {
  if (Value == Base.Value)
    return FPOptionsOverride();
  return getChangesSlow(Base);
}

/// Describes the kind of translation unit being processed.
enum TranslationUnitKind {
  /// The translation unit is a complete translation unit.
  TU_Complete,

  /// The translation unit is a prefix to a translation unit, and is
  /// not complete.
  TU_Prefix,

  /// The translation unit is a module.
  TU_Module,

  /// The translation unit is a is a complete translation unit that we might
  /// incrementally extend later.
  TU_Incremental
};

} // namespace clang

#endif // LLVM_CLANG_BASIC_LANGOPTIONS_H
