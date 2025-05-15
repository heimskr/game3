#ifdef __MINGW32__
// I hate my life right now

extern "C" {
	extern void _ZN9FastNoise22NewFromEncodedNodeTreeEPKcN8FastSIMD6eLevelE();
	extern void _ZN9FastNoise16SmartNodeManager12IncReferenceEy();
	extern void _ZN9FastNoise16SmartNodeManager12DecReferenceEyPvPFvS1_E();
	extern void SHCreateMemStream();
	const auto &__imp__ZN9FastNoise22NewFromEncodedNodeTreeEPKcN8FastSIMD6eLevelE = _ZN9FastNoise22NewFromEncodedNodeTreeEPKcN8FastSIMD6eLevelE;
	const auto &__imp__ZN9FastNoise16SmartNodeManager12IncReferenceEy = _ZN9FastNoise16SmartNodeManager12IncReferenceEy;
	const auto &__imp__ZN9FastNoise16SmartNodeManager12DecReferenceEyPvPFvS1_E = _ZN9FastNoise16SmartNodeManager12DecReferenceEyPvPFvS1_E;
	const auto &__imp_SHCreateMemStream = SHCreateMemStream;
}
#endif
