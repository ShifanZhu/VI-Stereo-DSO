#include "FullSystem/IMUPreintegrator.h"
#include "util/settings.h"

namespace dso
{
IMUPreintegrator::IMUPreintegrator(const IMUPreintegrator& pre):
    _delta_P(pre._delta_P), 
    _delta_V(pre._delta_V), 
    _delta_R(pre._delta_R),
    _J_P_Biasg(pre._J_P_Biasg), 
    _J_P_Biasa(pre._J_P_Biasa),
    _J_V_Biasg(pre._J_V_Biasg), 
    _J_V_Biasa(pre._J_V_Biasa),
    _J_R_Biasg(pre._J_R_Biasg),
    _cov_P_V_Phi(pre._cov_P_V_Phi),
    _delta_time(pre._delta_time)
{

}  

IMUPreintegrator::IMUPreintegrator()
{
    // delta measurements, position/velocity/rotation(matrix)
    _delta_P.setZero();    // P_k+1 = P_k + V_k*dt + R_k*a_k*dt*dt/2
    _delta_V.setZero();    // V_k+1 = V_k + R_k*a_k*dt
    _delta_R.setIdentity();    // R_k+1 = R_k*exp(w_k*dt).     note: Rwc, Rwc'=Rwc*[w_body]x

    // jacobian of delta measurements w.r.t bias of gyro/acc
    _J_P_Biasg.setZero();     // position / gyro
    _J_P_Biasa.setZero();     // position / acc
    _J_V_Biasg.setZero();     // velocity / gyro
    _J_V_Biasa.setZero();     // velocity / acc
    _J_R_Biasg.setZero();   // rotation / gyro

    // noise covariance propagation of delta measurements
    _cov_P_V_Phi.setZero();

    _delta_time = 0;
}

void IMUPreintegrator::reset()
{
    // delta measurements, position/velocity/rotation(matrix)
    _delta_P.setZero();    // P_k+1 = P_k + V_k*dt + R_k*a_k*dt*dt/2
    _delta_V.setZero();    // V_k+1 = V_k + R_k*a_k*dt
    _delta_R.setIdentity();    // R_k+1 = R_k*exp(w_k*dt).     note: Rwc, Rwc'=Rwc*[w_body]x

    // jacobian of delta measurements w.r.t bias of gyro/acc
    _J_P_Biasg.setZero();     // position / gyro
    _J_P_Biasa.setZero();     // position / acc
    _J_V_Biasg.setZero();     // velocity / gyro
    _J_V_Biasa.setZero();     // velocity / acc
    _J_R_Biasg.setZero();   // rotation / gyro

    // noise covariance propagation of delta measurements
    _cov_P_V_Phi.setZero();

    _delta_time = 0;

}

// incrementally update 1)delta measurements, 2)jacobians, 3)covariance matrix
// acc: acc_measurement - bias_a, last measurement!! not current measurement
// omega: gyro_measurement - bias_g, last measurement!! not current measurement
void IMUPreintegrator::update(const Vec3& omega, const Vec3& acc, const double& dt)
{
	LOG(INFO)<<"omega: \n"<<omega.transpose();
	LOG(INFO)<<"acc: \n"<<acc.transpose();
	LOG(INFO)<<"dt: \n"<<dt;
    double dt2 = dt*dt;

    Mat33 dR = Expmap(omega*dt);
    Mat33 Jr = JacobianR(omega*dt);

    // noise covariance propagation of delta measurements
    // err_k+1 = A*err_k + B*err_gyro + C*err_acc
    Mat33 I3x3 = Mat33::Identity();
    Mat99 A = Mat99::Identity();
    A.block<3,3>(6,6) = dR.transpose();
    A.block<3,3>(3,6) = -_delta_R*skew(acc)*dt;
    A.block<3,3>(0,6) = -0.5*_delta_R*skew(acc)*dt2;
    A.block<3,3>(0,3) = I3x3*dt;
	LOG(INFO)<<"A: \n"<<A;
    Mat93 Bg = Mat93::Zero();
    Bg.block<3,3>(6,0) = Jr*dt;
    Mat93 Ca = Mat93::Zero();
    Ca.block<3,3>(3,0) = _delta_R*dt;
    Ca.block<3,3>(0,0) = 0.5*_delta_R*dt2;
    _cov_P_V_Phi = A*_cov_P_V_Phi*A.transpose() +
        Bg*GyrCov*Bg.transpose() +
        Ca*AccCov*Ca.transpose();
	LOG(INFO)<<"_cov_P_V_Phi ========= : \n"<<_cov_P_V_Phi;


    // jacobian of delta measurements w.r.t bias of gyro/acc
    // update P first, then V, then R
    _J_P_Biasa += _J_V_Biasa*dt - 0.5*_delta_R*dt2;
    _J_P_Biasg += _J_V_Biasg*dt - 0.5*_delta_R*skew(acc)*_J_R_Biasg*dt2;
    _J_V_Biasa += -_delta_R*dt;
    _J_V_Biasg += -_delta_R*skew(acc)*_J_R_Biasg*dt;
    _J_R_Biasg = dR.transpose()*_J_R_Biasg - Jr*dt;

    // delta measurements, position/velocity/rotation(matrix)
    // update P first, then V, then R. because P's update need V&R's previous state
    _delta_P += _delta_V*dt + 0.5*_delta_R*acc*dt2;    // P_k+1 = P_k + V_k*dt + R_k*a_k*dt*dt/2
    _delta_V += _delta_R*acc*dt;
    _delta_R = normalizeRotationM(_delta_R*dR);  // normalize rotation, in case of numerical error accumulation


//    // noise covariance propagation of delta measurements
//    // err_k+1 = A*err_k + B*err_gyro + C*err_acc
//    Mat33 I3x3 = Mat33::Identity();
//    MatrixXd A = MatrixXd::Identity(9,9);
//    A.block<3,3>(6,6) = dR.transpose();
//    A.block<3,3>(3,6) = -_delta_R*skew(acc)*dt;
//    A.block<3,3>(0,6) = -0.5*_delta_R*skew(acc)*dt2;
//    A.block<3,3>(0,3) = I3x3*dt;
//    MatrixXd Bg = MatrixXd::Zero(9,3);
//    Bg.block<3,3>(6,0) = Jr*dt;
//    MatrixXd Ca = MatrixXd::Zero(9,3);
//    Ca.block<3,3>(3,0) = _delta_R*dt;
//    Ca.block<3,3>(0,0) = 0.5*_delta_R*dt2;
//    _cov_P_V_Phi = A*_cov_P_V_Phi*A.transpose() +
//        Bg*IMUData::getGyrMeasCov*Bg.transpose() +
//        Ca*IMUData::getAccMeasCov()*Ca.transpose();

    // delta time
    _delta_time += dt;

}
  
}