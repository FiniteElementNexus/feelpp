import sys
from feelpp.toolboxes.heat import *
from feelpp.mor import *
import feelpp

o=toolboxes_options("heat")
o.add(makeToolboxMorOptions())
e=feelpp.Environment(sys.argv,opts=o)

heatBox=heat(dim=2,order=1)
heatBox.init()
model = toolboxmor_2d()
model.setFunctionSpaces( Vh=heatBox.spaceTemperature())

def assembleDEIM(mu):
    for i in range(0,mu.size()):
        heatBox.addParameterInModelProperties(mu.parameterName(i),mu(i))
    heatBox.updateParameterValues()
    return heatBox.assembleRhs()

def assembleMDEIM(mu):
    for i in range(0,mu.size()):
        heatBox.addParameterInModelProperties(mu.parameterName(i),mu(i))
    heatBox.updateParameterValues()
    return heatBox.assembleMatrix()

model.setAssembleDEIM(fct=assembleDEIM)
model.setAssembleMDEIM(fct=assembleMDEIM)
model.initModel()

heatBoxDEIM=heat(dim=2,order=1)
meshDEIM = model.getDEIMReducedMesh()
heatBoxDEIM.setMesh(meshDEIM)
heatBoxDEIM.init()

def assembleOnlineDEIM(mu):
    for i in range(0,mu.size()):
        heatBoxDEIM.addParameterInModelProperties(mu.parameterName(i),mu(i))
    heatBoxDEIM.updateParameterValues()
    return heatBoxDEIM.assembleRhs()

model.setOnlineAssembleDEIM(assembleOnlineDEIM)

heatBoxMDEIM=heat(dim=2,order=1)
meshMDEIM = model.getMDEIMReducedMesh()
heatBoxMDEIM.setMesh(meshMDEIM)
heatBoxMDEIM.init()

def assembleOnlineMDEIM(mu):
    for i in range(0,mu.size()):
        heatBoxMDEIM.addParameterInModelProperties(mu.parameterName(i),mu(i))
    heatBoxMDEIM.updateParameterValues()
    return heatBoxMDEIM.assembleMatrix()

model.setOnlineAssembleMDEIM(assembleOnlineMDEIM)

model.postInitModel()
model.setInitialized(True)
crbmodel = crbmodel_toolboxmor_2d(model)
crb = crb_toolboxmor_2d(crbmodel)
crb.offline()

print("cool")
