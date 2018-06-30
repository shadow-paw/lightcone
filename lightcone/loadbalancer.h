#ifndef LIGHTCONE_LOADBALANCER_H__
#define LIGHTCONE_LOADBALANCER_H__

#include <atomic>

namespace lightcone {
// -----------------------------------------------------------
//! Load Balancer Interface
template <typename TOKEN>
class LoadBalancer {
 public:
    //! Setup load balancer
    //! \param[in] min minimum value to return for retain()
    //! \param[in] max maximum value to return for retain()
    //! \return true on success, false on fail with no side-effect.
    virtual bool setup(int min, int max) = 0;
    //! retain a value
    //! \param[in] token Hashable token used for load balancing logic.
    //! \return identitifer for the work load
    //! \sa release
    virtual int retain(const TOKEN& token) = 0;
    //! release a value
    //! \param[in] token Hashable token used for load balancing logic.
    //! \sa retain
    virtual void release(const TOKEN& token) = 0;
};
// -----------------------------------------------------------
//! Load balancer - Round Robin
template <typename TOKEN>
class LoadBalancerRR : public LoadBalancer<TOKEN> {
 public:
    bool setup  (int min, int max)   { m_min = min; m_max = max; m_next = min; m_mod = max - min +1; return true; }
    int  retain (const TOKEN& token) { return m_min + ((m_next++) % m_mod); }
    void release(const TOKEN& token) { }
 private:
    int m_min, m_max, m_mod;
    std::atomic<int> m_next;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_LOADBALANCER_H__
