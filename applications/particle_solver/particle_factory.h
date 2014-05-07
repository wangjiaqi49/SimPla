/*
 * particle_factory.h
 *
 *  Created on: 2014年2月11日
 *      Author: salmon
 */

#include <memory>
#include <string>

#include "../../src/fetl/primitives.h"
#include "../../src/particle/particle.h"
#include "../../src/particle/particle_base.h"
#include "fluid_cold_engine.h"
#include "pic_engine_default.h"
#include "pic_engine_deltaf.h"
#include "pic_engine_ggauge.h"

namespace simpla
{

template<typename Mesh, typename ...Args>
std::shared_ptr<ParticleBase<Mesh>> ParticleFactory(Args const & ...args)
{
	std::shared_ptr<ParticleBase<Mesh>> res(nullptr);

	if (res == nullptr)
		res = CreateParticle<Particle<PICEngineDefault<Mesh, true>>>(std::forward<Args const &>(args)...);

	if (res == nullptr)
		res = CreateParticle<Particle<PICEngineDefault<Mesh, false>>>(std::forward<Args const &>(args)...);

	if (res == nullptr)
		res = CreateParticle<Particle<PICEngineDeltaF<Mesh>>>(std::forward<Args const &>(args)...);

	if (res == nullptr)
		res = CreateParticle<Particle<PICEngineGGauge<Mesh, 4, true>>>(std::forward<Args const &>(args)...);

	if (res == nullptr)
		res = CreateParticle<Particle<PICEngineGGauge<Mesh, 16, true>>>(std::forward<Args const &>(args)...);

	if (res == nullptr)
		res = CreateParticle<Particle<PICEngineGGauge<Mesh, 32, true>>>(std::forward<Args const &>(args)...);

	if (res == nullptr)
		res = CreateParticle<Particle<ColdFluid<Mesh> >>(std::forward<Args const &>(args)...);

	if (res == nullptr)
		ERROR << "illegal particle configure";

	return res;
}

}
// namespace simpla

